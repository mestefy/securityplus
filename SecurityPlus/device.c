#include "device.h"
#include "process.h"
#include "helpers.h"
#include "communication.h"
#include "protect.h"

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
    NTSTATUS result;

    UNREFERENCED_PARAMETER(RegistryPath);

    DbgPrint("%wZ", &Intro);

    __debugbreak();

    ExInitializeResourceLite(&Resource);

    result = PsSetCreateProcessNotifyRoutineEx(NotifyProcessCreation, FALSE);
    switch (result)
    {
        case STATUS_SUCCESS:
        {
            DbgPrint("Method registered successfully\n");
            break;
        }
        case STATUS_ACCESS_DENIED:
        {
            DbgPrint("Access denied\n");
            break;
        }
        case STATUS_INVALID_PARAMETER:
        {
            DbgPrint("Invalid parameter\n");
            break;
        }
        default:
        {
            DbgPrint("Something unexpected happened\n");
        }
    }

    InitializeServer();

    DriverObject->DriverUnload = DriverUnload;

    return STATUS_SUCCESS;
}

void DriverUnload(_In_ PDRIVER_OBJECT DriverObject)
{
    NTSTATUS result;

    UNREFERENCED_PARAMETER(DriverObject);

    DeinitializeProcessProtection();

    result = PsSetCreateProcessNotifyRoutineEx(NotifyProcessCreation, TRUE);
    if (FALSE != NT_SUCCESS(result))
    {
        DbgPrint("Driver unloaded successfully\n");
    }
    else
    {
        DbgPrint("Driver unloaded with error\n");
    }

    DeinitializeServer();

    ExDeleteResourceLite(&Resource);

    DbgPrint("%wZ", &Outro);
}
