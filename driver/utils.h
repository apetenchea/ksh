#pragma once

#include <ntifs.h>
#include <windef.h>

typedef struct _FILE_PAIR_PARAM
{
    PCWSTR First;
    PCWSTR Second;
} FILE_PAIR_PARAM;

/*
 * The contents of Buffer should be "first|second"
 */
FILE_PAIR_PARAM ExtractFilePairFromBuffer(PWSTR Buffer, ULONG uBufferLen);

/*
 * Copies SourcePath to DestPath.
 */
NTSTATUS CopyFileHelper(PUNICODE_STRING SourcePath, PUNICODE_STRING DestPath);