#include "utils.h"

REG_PARAM ExtractRegDataFromBuffer(PVOID Buffer, ULONG uBufferLen)
{
    REG_PARAM params;
    params.dwType = 0;
    params.dwSize = 0;
    params.data = NULL;

    // Extract type and length
    if (uBufferLen < sizeof(DWORD) * 2)
    {
        return params;
    }
    params.dwType = ((DWORD *)Buffer)[0];
    params.dwSize = ((DWORD *)Buffer)[1];

    // Validation
    if (params.dwSize <= 0)
    {
        return params;
    }
    if (uBufferLen < sizeof(DWORD) * 2 + params.dwSize)
    {
        return params;
    }
    if (params.dwType != REG_DWORD && params.dwType != REG_QWORD && params.dwType != REG_SZ)
    {
        return params;
    }
    if (params.dwType == REG_DWORD && params.dwSize != sizeof(DWORD))
    {
        return params;
    }
    if (params.dwType == REG_QWORD && params.dwSize != sizeof(DWORD64))
    {
        return params;
    }

    params.data = (PBYTE)Buffer + sizeof(DWORD) * 2;
    return params;
}

STRINGS_PARAM ExtractStringsFromBuffer(PVOID Buffer, ULONG uBufferLen)
{
    STRINGS_PARAM params;
    params.First = NULL;
    params.Second = NULL;

    // Extract first string
    if (uBufferLen < sizeof(DWORD))
    {
        return params;
    }
    params.dwFirstSize = *(DWORD *)Buffer;
    if (uBufferLen < sizeof(DWORD) + params.dwFirstSize)
    {
        return params;
    }
    params.First = (PWSTR)((PCHAR)Buffer + sizeof(DWORD));

    // Extract second string
    if (uBufferLen < sizeof(DWORD) * 2 + params.dwFirstSize)
    {
        return params;
    }
    params.dwSecondSize = *(DWORD *)((PCHAR)Buffer + sizeof(DWORD) + params.dwFirstSize);
    if (uBufferLen < sizeof(DWORD) * 2 + params.dwFirstSize + params.dwSecondSize)
    {
        return params;
    }
    params.Second = (PWSTR)((PCHAR)Buffer + sizeof(DWORD) * 2 + params.dwFirstSize);

    return params;
}

NTSTATUS CopyFileHelper(PUNICODE_STRING SourcePath, PUNICODE_STRING DestPath)
{
    HANDLE hSrc;
    HANDLE hDest;

    IO_STATUS_BLOCK ioStatusRead;
    IO_STATUS_BLOCK ioStatusWrite;

    // Open source file
    OBJECT_ATTRIBUTES objAttrSrc;
    InitializeObjectAttributes(&objAttrSrc, SourcePath, OBJ_KERNEL_HANDLE, NULL, NULL);
    NTSTATUS status =
        ZwCreateFile(&hSrc, GENERIC_READ, &objAttrSrc, &ioStatusRead, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ,
                     FILE_OPEN, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    // Create/overwrite destination file
    OBJECT_ATTRIBUTES objAttrDest;
    InitializeObjectAttributes(&objAttrDest, DestPath, OBJ_KERNEL_HANDLE, NULL, NULL);
    status = ZwCreateFile(&hDest, GENERIC_WRITE, &objAttrDest, &ioStatusWrite, NULL, FILE_ATTRIBUTE_NORMAL, 0,
                          FILE_OVERWRITE_IF, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    // Read from source and write to destination
    CHAR buffer[4096];
    ULONG bytesRead;
    while (1)
    {
        status = ZwReadFile(hSrc, NULL, NULL, NULL, &ioStatusRead, buffer, sizeof(buffer), NULL, 0);

        bytesRead = (ULONG)ioStatusRead.Information;
        if (status == STATUS_END_OF_FILE && bytesRead == 0)
        {
            status = STATUS_SUCCESS;
            break;
        }
        if (!NT_SUCCESS(status) || bytesRead == 0)
        {
            break;
        }

        status = ZwWriteFile(hDest, NULL, NULL, NULL, &ioStatusWrite, buffer, bytesRead, NULL, 0);
        if (!NT_SUCCESS(status))
        {
            break;
        }
        if (ioStatusWrite.Information != bytesRead)
        {
            status = STATUS_INTERNAL_ERROR;
            break;
        }
    }

    ZwClose(hSrc);
    ZwClose(hDest);

    return status;
}

NTSTATUS RemoveFileHelper(PUNICODE_STRING FilePath)
{
    OBJECT_ATTRIBUTES objAttr;
    InitializeObjectAttributes(&objAttr, FilePath, OBJ_KERNEL_HANDLE, NULL, NULL);
    return ZwDeleteFile(&objAttr);
}