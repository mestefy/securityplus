#pragma once

#include <ntddk.h>

void NotifyProcessCreation(_Inout_ PEPROCESS Process, _In_ HANDLE ProcessId, _In_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo);

ERESOURCE Resource;
