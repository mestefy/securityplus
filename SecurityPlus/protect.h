#pragma once

#include <ntddk.h>

void InitializeProcessProtection(_In_ HANDLE ProcessId);

void DeinitializeProcessProtection();

OB_PREOP_CALLBACK_STATUS PreProtectProcess(_In_ PVOID RegistrationContext, _Inout_ POB_PRE_OPERATION_INFORMATION OperationInformation);

OB_PREOP_CALLBACK_STATUS PreProtectThread(_In_ PVOID RegistrationContext, _Inout_ POB_PRE_OPERATION_INFORMATION OperationInformation);
