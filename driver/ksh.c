#include "ksh.h"
#include "utils.h"

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
    UNREFERENCED_PARAMETER(pRegistryPath);

    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT pDeviceObject = NULL;

    // DosDeviceName will be used to access the device from user mode.
    UNICODE_STRING DosDeviceName;
    RtlInitUnicodeString(&DosDeviceName, DOS_DEVICE_NAME);

    // Create the DeviceObject, an endpoint point for communication between user-mode and kernel-mode
    UNICODE_STRING DeviceName;
    RtlInitUnicodeString(&DeviceName, DEVICE_NAME);
    NtStatus = IoCreateDevice(pDriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE,
                              &pDeviceObject);
    if (!NT_SUCCESS(NtStatus))
    {
        return NtStatus;
    }

    // Set up the driver's dispatch table.
    for (DWORD index = 0; index < IRP_MJ_MAXIMUM_FUNCTION; ++index)
    {
        pDriverObject->MajorFunction[index] = UnsupportedFunction;
    }
    pDriverObject->DriverUnload = Unload;
    pDriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;

    // The I/O manager will use buffered I/O for I/O requests.
    pDeviceObject->Flags |= DO_BUFFERED_IO;
    // Mark device as being initialized.
    pDeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);

    // Create a user-visible symbolic link.
    NtStatus = IoCreateSymbolicLink(&DosDeviceName, &DeviceName);
    if (!NT_SUCCESS(NtStatus))
    {
        IoDeleteDevice(pDeviceObject);
    }

    return NtStatus;
}

VOID Unload(PDRIVER_OBJECT pDriverObject)
{
    UNICODE_STRING DosDeviceName;
    RtlInitUnicodeString(&DosDeviceName, DOS_DEVICE_NAME);
    if (!NT_SUCCESS(IoDeleteSymbolicLink(&DosDeviceName)))
    {
        DbgPrint("IoDeleteSymbolicLink failed!");
    }
    IoDeleteDevice(pDriverObject->DeviceObject);
}

NTSTATUS DispatchCreate(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
    UNREFERENCED_PARAMETER(pDeviceObject);
    UNREFERENCED_PARAMETER(pIrp);
    return STATUS_SUCCESS;
}

NTSTATUS DispatchClose(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
    UNREFERENCED_PARAMETER(pDeviceObject);
    UNREFERENCED_PARAMETER(pIrp);
    return STATUS_SUCCESS;
}

NTSTATUS UnsupportedFunction(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
    UNREFERENCED_PARAMETER(pDeviceObject);
    UNREFERENCED_PARAMETER(pIrp);
    return STATUS_NOT_SUPPORTED;
}

NTSTATUS DispatchDeviceControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
    UNREFERENCED_PARAMETER(pDeviceObject);

    NTSTATUS status = STATUS_SUCCESS;

    // Entry in the I/O stack associated with this IRP.
    PIO_STACK_LOCATION pIoStackLocation = IoGetCurrentIrpStackLocation(pIrp);
    PVOID pSystemBuffer = pIrp->AssociatedIrp.SystemBuffer;

    switch (pIoStackLocation->Parameters.DeviceIoControl.IoControlCode)
    {
    case IOCTL_KSH_TEST:
        status = TestDriver(pIoStackLocation, pSystemBuffer);
        break;
    case IOCTL_KSH_PKILL:
        status = KillProcess(pIoStackLocation, pSystemBuffer);
        break;
    case IOCTL_KSH_REMOVE_FILE:
        status = RemoveFile(pIoStackLocation, pSystemBuffer);
        break;
    case IOCTL_KSH_COPY_FILE:
        status = CopyFile(pIoStackLocation, pSystemBuffer);
        break;
    case IOCTL_KSH_MOVE_FILE:
        status = MoveFile(pIoStackLocation, pSystemBuffer);
        break;
    default:
        status = STATUS_INVALID_PARAMETER_1;
        break;
    }

    RtlZeroMemory(pSystemBuffer, pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength);
    if (!NT_SUCCESS(status))
    {
        *(DWORD *)pSystemBuffer = (DWORD)1;
    }

    pIrp->IoStatus.Information = sizeof(DWORD);
    pIrp->IoStatus.Status = status;

    // Return the IRP to the I/O manager.
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    return status;
}

NTSTATUS KillProcess(PIO_STACK_LOCATION pIoStackLocation, PVOID pSystemBuffer)
{
    NTSTATUS status = STATUS_SUCCESS;
    if (pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength == sizeof(DWORD))
    {
        DWORD dwPid;
        RtlCopyMemory(&dwPid, pSystemBuffer, sizeof(dwPid));

        PEPROCESS eProcess = NULL;
        status = PsLookupProcessByProcessId((HANDLE)dwPid, &eProcess);
        if (NT_SUCCESS(status))
        {
            __try
            {
                HANDLE hProcess = NULL;
                status = ObOpenObjectByPointer(eProcess, OBJ_KERNEL_HANDLE, NULL, KEY_ALL_ACCESS, NULL, KernelMode,
                                               &hProcess);
                if (NT_SUCCESS(status) && hProcess != NULL)
                {
                    status = ZwTerminateProcess(hProcess, STATUS_FAIL_FAST_EXCEPTION);
                    ZwClose(hProcess);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                status = STATUS_INTERNAL_ERROR;
            }
        }
    }
    else
    {
        status = STATUS_INVALID_PARAMETER_1;
    }
    return status;
}

NTSTATUS TestDriver(PIO_STACK_LOCATION pIoStackLocation, PVOID pSystemBuffer)
{
    NTSTATUS status = STATUS_SUCCESS;
    if (pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength == sizeof(DWORD))
    {
        DWORD dwPid;
        RtlCopyMemory(&dwPid, pSystemBuffer, sizeof(dwPid));
        if (dwPid != 1)
        {
            status = STATUS_INVALID_PARAMETER_1;
        }
    }
    else
    {
        status = STATUS_INVALID_PARAMETER_1;
    }
    return status;
}

NTSTATUS RemoveFile(PIO_STACK_LOCATION pIoStackLocation, PVOID pSystemBuffer)
{
    if (pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength == 0)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    UNICODE_STRING uniFileName;
    RtlInitUnicodeString(&uniFileName, (PCWSTR)pSystemBuffer);

    return RemoveFileHelper(&uniFileName);
}

NTSTATUS CopyFile(PIO_STACK_LOCATION pIoStackLocation, PVOID pSystemBuffer)
{
    if (pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength == 0)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    STRINGS_PARAM files =
        ExtractStringsFromBuffer(pSystemBuffer, pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength);
    if (files.First == NULL || files.Second == NULL)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    UNICODE_STRING uniSrc;
    RtlInitUnicodeString(&uniSrc, files.First);
    UNICODE_STRING uniDest;
    RtlInitUnicodeString(&uniDest, files.Second);

    return CopyFileHelper(&uniSrc, &uniDest);
}

NTSTATUS MoveFile(PIO_STACK_LOCATION pIoStackLocation, PVOID pSystemBuffer)
{
    if (pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength == 0)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    STRINGS_PARAM files =
        ExtractStringsFromBuffer(pSystemBuffer, pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength);
    if (files.First == NULL || files.Second == NULL)
    {
        return STATUS_INVALID_PARAMETER_1;
    }

    UNICODE_STRING uniSrc;
    RtlInitUnicodeString(&uniSrc, files.First);
    UNICODE_STRING uniDest;
    RtlInitUnicodeString(&uniDest, files.Second);

    NTSTATUS status = CopyFileHelper(&uniSrc, &uniDest);
    if (!NT_SUCCESS(status))
    {
        return status;
    }
    return RemoveFileHelper(&uniSrc);
}
