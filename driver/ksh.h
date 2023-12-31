#pragma once

#include <ntifs.h>
#include <windef.h>

#define DEVICE_NAME L"\\Device\\KSH"
#define DOS_DEVICE_NAME L"\\DosDevices\\KSH"

// Called when the driver is loaded.
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath);
// Called when the driver is unloaded.
DRIVER_UNLOAD Unload;
// Called when a HANDLE is requested (using CreateFile, for example).
DRIVER_DISPATCH DispatchCreate;
// Called when the last HANDLE has been closed and released (CloseHandle).
DRIVER_DISPATCH DispatchClose;
// Called when a request containing an I/O control code is passed to the device driver.
DRIVER_DISPATCH DispatchDeviceControl;
// Used to indicate unsupported functionality.
DRIVER_DISPATCH UnsupportedFunction;

/*
 * INIT - procedure is needed only for initialization, after which it can be removed from memory.
 * PAGE - code can be paged when the device is not active.
 */
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, Unload)
#pragma alloc_text(PAGE, DispatchCreate)
#pragma alloc_text(PAGE, DispatchClose)
#pragma alloc_text(PAGE, UnsupportedFunction)
#pragma alloc_text(PAGE, DispatchDeviceControl)

/*
 * Control code for killing a process.
 * The PID is passed as a DWORD. Returns 0 on success, 1 on failure.
 */
#define IOCTL_KSH_PKILL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
NTSTATUS KillProcess(PIO_STACK_LOCATION pIoStackLocation, PVOID pSystemBuffer);

/*
 * Control code for performing a basic test.
 * Expects 1 to be passed as a DWORD. Returns 0 on success, 1 on failure.
 */
#define IOCTL_KSH_TEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
NTSTATUS TestDriver(PIO_STACK_LOCATION pIoStackLocation, PVOID pSystemBuffer);

/*
 * Control code for removing a file.
 * Expects the file name to be passsed as a unicode string. Returns 0 on success, 1 on failure.
 */
#define IOCTL_KSH_REMOVE_FILE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
NTSTATUS RemoveFile(PIO_STACK_LOCATION pIoStackLocation, PVOID pSystemBuffer);

/*
 * Control code for copying a file.
 * Expects file names to be passed as wide-strings, preceded by their length (including the NULL-terminating
 *character).
 * {
 *	DWORD size;
 *	WSTR string;
 * }
 * Returns 0 on success, 1 on failure.
 */
#define IOCTL_KSH_COPY_FILE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
NTSTATUS CopyFile(PIO_STACK_LOCATION pIoStackLocation, PVOID pSystemBuffer);

/*
 * Control code for moving a file.
 * Expects file names to be passed as wide-strings, preceded by their length (including the NULL-terminating
 *character).
 * {
 *	DWORD size;
 *	WSTR string;
 * }
 * Returns 0 on success, 1 on failure.
 */
#define IOCTL_KSH_MOVE_FILE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
NTSTATUS MoveFile(PIO_STACK_LOCATION pIoStackLocation, PVOID pSystemBuffer);

/*
 * Control code for editing a registry key. If the key is not found, it is automatically created.
 * Expects the buffer:
 * - DWORD registry data type
 * - DWORD size of the data
 * - PVOID (actual data)
 * - DWORD size of the registry key
 * - WSTR registry key
 * - DWORD size of the registry value
 * - WSTR registry value
 * Returns 0 on success, 1 on failure.
 */
#define IOCTL_KSH_REGEDIT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
NTSTATUS Regedit(PIO_STACK_LOCATION pIoStackLocation, PVOID pSystemBuffer);