#pragma once

#include <ntdef.h>

PUNICODE_STRING AllocateUnicodeString(int Size);

void DeallocateUnicodeString(PUNICODE_STRING String);

PUNICODE_STRING GetProcessName1(_In_ PVOID Process);

void GetFileObjectName(_In_ PVOID FileObject, _Out_ PUNICODE_STRING* DosFileName);

void GetProcessName(_In_ PVOID Process, _Out_ PUNICODE_STRING* DosFileName);
