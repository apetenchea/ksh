#pragma once

#include <ntifs.h>
#include <windef.h>

typedef struct _STRINGS_PARAM
{
    DWORD dwFirstSize;
    PCWSTR First;
    DWORD dwSecondSize;
    PCWSTR Second;
} STRINGS_PARAM;

typedef struct _REG_PARAM
{
    DWORD dwType;
    DWORD dwSize;
    PVOID data;
} REG_PARAM;

/*
 * The buffer is expected to contain integers, followed by the actual data.
 */
REG_PARAM ExtractRegDataFromBuffer(PVOID Buffer, ULONG uBufferLen);

/*
 * The buffer may contain up to two strings, encoded as [DWORD size][WSTR string]
 */
STRINGS_PARAM ExtractStringsFromBuffer(PVOID Buffer, ULONG uBufferLen);

/*
 * Copies SourcePath to DestPath.
 */
NTSTATUS CopyFileHelper(PUNICODE_STRING SourcePath, PUNICODE_STRING DestPath);

/*
 * Remove file pointer by FilePath.
 */
NTSTATUS RemoveFileHelper(PUNICODE_STRING FilePath);