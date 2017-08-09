#include "protect.h"

#include <wdm.h>

typedef struct _OB_GLOBAL
{
    HANDLE ProtectedProcessId;
    PVOID ObRegistrationHandle;
    UNICODE_STRING Altitude;
    OB_OPERATION_REGISTRATION Operations[2];
    USHORT OperationsCount;
}OB_GLOBAL, *POB_GLOBAL;

#define PROCESS_TERMINATE (0x0001)
#define PROCESS_SUSPEND_RESUME (0x0800)

#define THREAD_TERMINATE (0x0001)
#define THREAD_SUSPEND_RESUME (0x0002)

static const ACCESS_MASK ProcessMask = PROCESS_TERMINATE | PROCESS_SUSPEND_RESUME;
static const ACCESS_MASK ThreadMask = THREAD_TERMINATE | THREAD_SUSPEND_RESUME;

OB_GLOBAL GlobalRegistrationData = { 0 };

void InitializeProcessProtection(_In_ HANDLE ProcessId)
{
    __debugbreak();

    NTSTATUS result;
    OB_CALLBACK_REGISTRATION registration = { 0 };

    __try
    {
        GlobalRegistrationData.ProtectedProcessId = ProcessId;
        RtlInitUnicodeString(&GlobalRegistrationData.Altitude, L"0");
        GlobalRegistrationData.OperationsCount = 2;

        GlobalRegistrationData.Operations[0].ObjectType = PsProcessType;
        GlobalRegistrationData.Operations[0].Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
        GlobalRegistrationData.Operations[0].PreOperation = PreProtectProcess;
        GlobalRegistrationData.Operations[0].PostOperation = NULL;

        GlobalRegistrationData.Operations[1].ObjectType = PsThreadType;
        GlobalRegistrationData.Operations[1].Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
        GlobalRegistrationData.Operations[1].PreOperation = PreProtectThread;
        GlobalRegistrationData.Operations[1].PostOperation = NULL;

        registration.Version = OB_FLT_REGISTRATION_VERSION;
        registration.OperationRegistrationCount = GlobalRegistrationData.OperationsCount;
        registration.OperationRegistration = GlobalRegistrationData.Operations;
        registration.RegistrationContext = &GlobalRegistrationData;
        registration.Altitude = GlobalRegistrationData.Altitude;

        result = ObRegisterCallbacks(&registration, &GlobalRegistrationData.ObRegistrationHandle);
        if (FALSE == NT_SUCCESS(result))
        {
            DbgPrint("Failed to register callback with error %d\n", result);

            switch (result)
            {
                case STATUS_FLT_INSTANCE_ALTITUDE_COLLISION:
                {
                    DbgPrint("Altitude issue\n");
                    break;
                }

                case STATUS_INVALID_PARAMETER:
                {
                    DbgPrint("Invalid parameter\n");
                    break;
                }

                case STATUS_ACCESS_DENIED:
                {
                    DbgPrint("Access denied\n");
                    break;
                }
            }

            __leave;
        }
    }
    __finally
    {
    }
}

void DeinitializeProcessProtection()
{
    __try
    {
        if (NULL != GlobalRegistrationData.ObRegistrationHandle)
        {
            ObUnRegisterCallbacks(GlobalRegistrationData.ObRegistrationHandle);
        }
    }
    __finally
    {
    }
}

OB_PREOP_CALLBACK_STATUS PreProtectProcess(_In_ PVOID RegistrationContext, _Inout_ POB_PRE_OPERATION_INFORMATION OperationInformation)
{
    HANDLE targetProcessId = NULL;
    HANDLE currentProcessId = NULL;
    PACCESS_MASK desiredAccess = NULL;

    POB_GLOBAL globalData = (POB_GLOBAL)RegistrationContext;

    __try
    {
        // Operation was requested by kernel, allow
        /*if (1 == OperationInformation->KernelHandle)
        {
            __leave;
        }*/

        targetProcessId = PsGetProcessId((PEPROCESS)OperationInformation->Object);
        // The target process is not the protected process, skipping
        if (targetProcessId != globalData->ProtectedProcessId)
        {
            __leave;
        }

        currentProcessId = PsGetProcessId(PsGetCurrentProcess());
        // Protected process requested access
        if (currentProcessId == globalData->ProtectedProcessId)
        {
            __leave;
        }

        DbgPrint("PreProtectProcess Current process [%d] Target process[%d] - Protected process[%d]\n", currentProcessId, targetProcessId, globalData->ProtectedProcessId);

        __debugbreak();
        switch (OperationInformation->Operation)
        {
            case OB_OPERATION_HANDLE_CREATE:
            {
                desiredAccess = &OperationInformation->Parameters->CreateHandleInformation.DesiredAccess;
                break;
            }
            case OB_OPERATION_HANDLE_DUPLICATE:
            {
                desiredAccess = &OperationInformation->Parameters->DuplicateHandleInformation.DesiredAccess;
                break;
            }
        }

        *desiredAccess &= ~ProcessMask;
    }
    __finally
    {
    }

    return OB_PREOP_SUCCESS;
}

OB_PREOP_CALLBACK_STATUS PreProtectThread(_In_ PVOID RegistrationContext, _Inout_ POB_PRE_OPERATION_INFORMATION OperationInformation)
{
    HANDLE targetProcessId = NULL;
    HANDLE currentProcessId = NULL;

    PACCESS_MASK desiredAccess = NULL;

    POB_GLOBAL globalData = (POB_GLOBAL)RegistrationContext;

    __try
    {
        // Operation was requested by kernel, allow
        /*if (1 == OperationInformation->KernelHandle)
        {
            __leave;
        }*/

        targetProcessId = PsGetThreadProcessId((PETHREAD)OperationInformation->Object);
        // The target process is not the protected process, skipping
        if (targetProcessId != globalData->ProtectedProcessId)
        {
            __leave;
        }

        currentProcessId = PsGetProcessId(PsGetCurrentProcess());
        // Protected process requested access, allow
        if (currentProcessId == globalData->ProtectedProcessId)
        {
            __leave;
        }

        DbgPrint("PreProtectThread Current process [%d] Target process[%d] - Protected process[%d]\n", currentProcessId, targetProcessId, globalData->ProtectedProcessId);
        __debugbreak();
        switch (OperationInformation->Operation)
        {
            case OB_OPERATION_HANDLE_CREATE:
            {
                desiredAccess = &OperationInformation->Parameters->CreateHandleInformation.DesiredAccess;
                break;
            }
            case OB_OPERATION_HANDLE_DUPLICATE:
            {
                desiredAccess = &OperationInformation->Parameters->DuplicateHandleInformation.DesiredAccess;
                break;
            }
        }

        *desiredAccess &= ~ThreadMask;
    }
    __finally
    {
    }

    return OB_PREOP_SUCCESS;
}
