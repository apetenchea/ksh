#include "utils.h"

FILE_PAIR_PARAM ExtractFilePairFromBuffer(PWSTR Buffer, ULONG uBufferLen)
{
    FILE_PAIR_PARAM params;
    params.First = NULL;
    params.Second = NULL;

    PWSTR separator = Buffer;
    ULONG guard = 0;

    // Search for the '|' separator. It must not be one of the last two characters, because otherwise the second file
    // name would be NULL.
    while (guard < uBufferLen - 1 && *separator != L'\0' && *separator != L'|')
    {
        ++separator;
        ++guard;
    }
    if (guard >= uBufferLen - 1 || *separator != L'|')
    {
        return params;
    }

    *separator = L'\0';
    if (*(++separator) == L'\0')
    {
        return params;
    }

    params.First = Buffer;
    params.Second = separator;
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