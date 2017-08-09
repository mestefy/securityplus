#pragma once

#include <ntddk.h>

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath);

void DriverUnload(_In_ PDRIVER_OBJECT DriverObject);

UNICODE_STRING Intro = RTL_CONSTANT_STRING(L"########################\n");

UNICODE_STRING Outro = RTL_CONSTANT_STRING(L"========================\n");