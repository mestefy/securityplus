#include "process.h"
#include "communication.h"

#include "defines.h"
#include "helpers.h"

ERESOURCE Resource;

void NotifyProcessCreation(_Inout_ PEPROCESS Process, _In_ HANDLE ProcessId, _In_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo)
{
    PUNICODE_STRING fileName;
    SEND_CONTEXT sendContext;

    NTSTATUS status;

    UCHAR operationType;
    PROCESS_VALIDATION_RESPONSE result = {0};

    fileName = NULL;

    __try
    {
        // Process creation
        if (NULL != CreateInfo)
        {
            if (NULL == CreateInfo->FileObject)
            {
                __debugbreak();
                GetProcessName(Process, &fileName);
                DbgPrint("********Process [%d] started: [INVALID FILE_OBJECT] %wZ\n", ProcessId, fileName);
            }
            else
            {
                GetFileObjectName(CreateInfo->FileObject, &fileName);
                DbgPrint("********Process [%d] started: %wZ\n", ProcessId, fileName);
            }

            operationType = PROCESS_CREATE;
        }
        // Process termination
        else
        {
            GetProcessName(Process, &fileName);
            DbgPrint("-----------Process [%d] ended: %wZ\n", ProcessId, fileName);
            operationType = PROCESS_CLOSE;
        }

        if (NULL != Global.ClientSocket)
        {
            // Initialize
            status = InitializeSendContextFromString(&sendContext, fileName, operationType, ProcessId);
            if (FALSE == NT_SUCCESS(status))
            {
                DbgPrint("Error on send context initialization\n");
                __leave;
            }

            // Locking
            ExAcquireResourceExclusiveLite(&Resource, TRUE);

            // Recheck
            if (NULL != Global.ClientSocket)
            {
                status = SendData(Global.ClientSocket, &sendContext);
                if (FALSE == NT_SUCCESS(status))
                {
                    DbgPrint("Error on send\n");
                    status = CloseSocket(Global.ClientSocket);
                    if (FALSE == NT_SUCCESS(status))
                    {
                        DbgPrint("Error on Close client socket\n");
                    }

                    Global.ClientSocket = NULL;
                }
                else
                {
                    status = ReceiveData(Global.ClientSocket, &result);
                    if (FALSE == NT_SUCCESS(status))
                    {
                        DbgPrint("Error on receive\n");
                        status = CloseSocket(Global.ClientSocket);
                        if (FALSE == NT_SUCCESS(status))
                        {
                            DbgPrint("Error on Close client socket\n");
                        }

                        Global.ClientSocket = NULL;
                    }
                    else
                    {
                        if (PROCESS_OPERATION_DENY == result.ProcessOperationResult)
                        {
                            CreateInfo->CreationStatus = STATUS_ACCESS_DENIED;
                        }
                    }
                }
            }

            // Unlocking
            ExReleaseResourceLite(&Resource);

            // Deinitialize
            DeinitializeSendContext(&sendContext);
        }
    }
    __finally
    {
        if (NULL != fileName)
        {
            ExFreePoolWithTag(fileName, FILE_NAME_TAG);
        }
    }
}
