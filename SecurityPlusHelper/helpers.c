#include "helpers.h"

#include "defines.h"

#include <ntifs.h>

typedef NTSTATUS(*QUERY_INFO_PROCESS)
(
    _In_ HANDLE ProcessHandle,
    _In_ PROCESSINFOCLASS ProcessInformationClass,
    __out_bcount(ProcessInformationLength) PVOID ProcessInformation,
    _In_ ULONG ProcessInformationLength,
    _Out_opt_ PULONG ReturnLength
    );

typedef NTSTATUS(*REFERENCE_PROCESS_FILEPOINTER)(_In_ PEPROCESS, _Out_ PFILE_OBJECT *);

QUERY_INFO_PROCESS ZwQueryInformationProcess;
REFERENCE_PROCESS_FILEPOINTER PsReferenceProcessFilePointer;

PUNICODE_STRING AllocateUnicodeString(_In_ int Size)
{
    PUNICODE_STRING string;

    UNREFERENCED_PARAMETER(Size);

    string = NULL;

    return string;
}

void DeallocateUnicodeString(_In_ PUNICODE_STRING String)
{
    if (NULL == String)
    {
        DbgPrint("String is null, cannot deallocate\n");
    }
    else
    {

    }
}

PUNICODE_STRING GetProcessName1(_In_ PVOID Process)
{
    NTSTATUS result;

    HANDLE processHandle;

    PUNICODE_STRING processName;

    ULONG processNameLength;

    PCHAR buffer;

    processNameLength = 0;
    buffer = NULL;
    result = STATUS_SUCCESS;
    processHandle = NULL;
    processName = NULL;

    //	__debugbreak();

    if (NULL == ZwQueryInformationProcess)
    {
        UNICODE_STRING routineName;

        RtlInitUnicodeString(&routineName, L"ZwQueryInformationProcess");

#pragma warning(disable:4055)
        ZwQueryInformationProcess = (QUERY_INFO_PROCESS)MmGetSystemRoutineAddress(&routineName);
    }

    do
    {
        if (NULL == ZwQueryInformationProcess)
        {
            DbgPrint("Could not resolve ZwQueryInformationProcess\n");
            break;
        }

        if (NULL != Process)
        {
            result = ObOpenObjectByPointer(Process, 0, NULL, 0, 0, KernelMode, &processHandle);
            if (FALSE == NT_SUCCESS(result))
            {
                DbgPrint("Failed to retrieve process handle\n");
                break;
            }
        }

        result = ZwQueryInformationProcess(processHandle, ProcessImageFileName, NULL, 0, &processNameLength);
        if (FALSE == NT_SUCCESS(result) && STATUS_INFO_LENGTH_MISMATCH != result)
        {
            DbgPrint("Failed to retrieve process name length. Error %X\n", result);
            break;
        }

        buffer = ExAllocatePoolWithTag(PagedPool, processNameLength, PROCESS_STRING_TAG);
        if (NULL == buffer)
        {
            DbgPrint("Not enough resources\n");
            break;
        }

        result = ZwQueryInformationProcess(processHandle, ProcessImageFileName, buffer, processNameLength, &processNameLength);
        if (FALSE == NT_SUCCESS(result))
        {
            DbgPrint("Failed to retrieve process name\n");
        }
        else
        {
            DbgPrint("Process created: %wZ\n", (PUNICODE_STRING)buffer);
        }
    }
    while (FALSE);

    if (NULL != processHandle)
    {
        ZwClose(processHandle);
    }

    if (NULL != buffer)
    {
        ExFreePoolWithTag(buffer, PROCESS_STRING_TAG);
    }

    return processName;
}

void GetFileObjectName(_In_ PVOID FileObject, _Out_ PUNICODE_STRING* DosFileName)
{
    NTSTATUS result;

    POBJECT_NAME_INFORMATION nameInformation;

    nameInformation = NULL;
    *DosFileName = NULL;

    //__debugbreak();

    __try
    {
        result = IoQueryFileDosDeviceName((PFILE_OBJECT)FileObject, &nameInformation);
        if (FALSE == NT_SUCCESS(result))
        {
            DbgPrint("Failed to retrieve name from file object. Error %X\n", result);
        }
        else
        {
            if (PASSIVE_LEVEL == KeGetCurrentIrql())
            {
                ULONG fileNameSize;

                fileNameSize = nameInformation->Name.Length + sizeof(UNICODE_STRING);

                *DosFileName = ExAllocatePoolWithTag(NonPagedPool, fileNameSize, FILE_NAME_TAG);
                if (NULL != *DosFileName)
                {
                    RtlCopyMemory(*DosFileName, &nameInformation->Name, fileNameSize);
                }
            }
        }
    }
    __finally
    {
        if (NULL != nameInformation)
        {
            ExFreePool(nameInformation);
        }
    }
}

void GetProcessName(_In_ PVOID Process, _Out_ PUNICODE_STRING* DosFileName)
{
    NTSTATUS result;
    PFILE_OBJECT fileObject;

    result = STATUS_SUCCESS;
    *DosFileName = NULL;
    fileObject = NULL;

    // __debugbreak();

    __try
    {
        if (NULL == PsReferenceProcessFilePointer)
        {
            UNICODE_STRING routineName;

            RtlInitUnicodeString(&routineName, L"PsReferenceProcessFilePointer");

#pragma warning(disable:4055)
            PsReferenceProcessFilePointer = (REFERENCE_PROCESS_FILEPOINTER)MmGetSystemRoutineAddress(&routineName);
        }

        if (NULL == PsReferenceProcessFilePointer)
        {
            DbgPrint("Failed to retrieve PsReferenceProcessFilePointer\n");
            __leave;
        }

        result = PsReferenceProcessFilePointer((PEPROCESS)Process, &fileObject);
        if (FALSE == NT_SUCCESS(result))
        {
            DbgPrint("Failed to retrieve file object\n");
            __leave;
        }

        GetFileObjectName(fileObject, DosFileName);
    }
    __finally
    {
        if (NULL != fileObject)
        {
            //			ObDereferenceObject(fileObject);
            fileObject = NULL;
        }
    }
}
