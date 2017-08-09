#include "communication.h"
#include "protect.h"

#include <ntstrsafe.h>

const WSK_CLIENT_DISPATCH WskAppDispatch = { MAKE_WSK_VERSION(1, 0), 0, NULL };

NTSTATUS IrpComplete(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    PCALLBACK_RESULT callbackResult;

    callbackResult = (PCALLBACK_RESULT)Context;

    // __debugbreak();

    __try
    {
        if (NULL == callbackResult)
        {
            DbgPrint("Context was NULL in IrpComplete\n");
            __leave;
        }

        callbackResult->Result = Irp->IoStatus.Status;
        if (FALSE != NT_SUCCESS(callbackResult->Result))
        {
            DbgPrint("Irp completed successfully\n");
        }

        KeSetEvent(&callbackResult->Event, 0, FALSE);
    }
    __finally
    {

    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS CreateSocketComplete(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    PCALLBACK_RESULT callbackResult;

    callbackResult = (PCALLBACK_RESULT)Context;

    // __debugbreak();

    __try
    {
        if (NULL == callbackResult)
        {
            DbgPrint("Context is NULL\n");
            __leave;
        }

        callbackResult->Result = Irp->IoStatus.Status;
        if (FALSE != NT_SUCCESS(callbackResult->Result))
        {
            DbgPrint("Socket created successfully\n");
            Global.ListeningSocket = (PWSK_SOCKET)Irp->IoStatus.Information;
        }

        KeSetEvent(&callbackResult->Event, 2, FALSE);
    }
    __finally
    {

    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS AcceptSocketComplete(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    PCALLBACK_RESULT callbackResult;

    callbackResult = (PCALLBACK_RESULT)Context;

    // __debugbreak();

    __try
    {
        if (NULL == callbackResult)
        {
            DbgPrint("Context was NULL in AcceptSocketComplete\n");
            __leave;
        }

        callbackResult->Result = Irp->IoStatus.Status;
        if (FALSE != NT_SUCCESS(callbackResult->Result))
        {
            DbgPrint("Accept socket completed successfully\n");
            Global.ClientSocket = (PWSK_SOCKET)Irp->IoStatus.Information;
            // CloseSocket((PWSK_SOCKET)Irp->IoStatus.Information);
        }

        KeSetEvent(&callbackResult->Event, 2, FALSE);
    }
    __finally
    {

    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

void CreateSocket(PIRP Irp, PCALLBACK_RESULT Context)
{
    const WSK_PROVIDER_DISPATCH *dispatch = Global.Provider.Dispatch;

    // __debugbreak();

    __try
    {
        if (NULL == Irp)
        {
            DbgPrint("Irp in CreateSocket is NULL\n");
            __leave;
        }

        if (NULL == Context)
        {
            DbgPrint("Callback context in CreateSocket is NULL\n");
            __leave;
        }

        IoSetCompletionRoutine(Irp, CreateSocketComplete, Context, TRUE, TRUE, TRUE);

        dispatch->WskSocket(Global.Provider.Client, AF_INET6, SOCK_STREAM, IPPROTO_TCP, WSK_FLAG_LISTEN_SOCKET, NULL, &Global.SocketDispatch, NULL, NULL, NULL, Irp);

        KeWaitForSingleObject(&Context->Event, Executive, KernelMode, FALSE, NULL);

        IoReuseIrp(Irp, STATUS_UNSUCCESSFUL);
    }
    __finally
    {
        DbgPrint("Exiting CreateSocket\n");
    }
}

NTSTATUS CloseSocket(PWSK_SOCKET Socket)
{
    PWSK_PROVIDER_BASIC_DISPATCH dispatch;

    PIRP irp;

    CALLBACK_RESULT callbackResult = { 0 };

    // __debugbreak();

    dispatch = (PWSK_PROVIDER_BASIC_DISPATCH)(Socket->Dispatch);

    KeInitializeEvent(&callbackResult.Event, SynchronizationEvent, FALSE);

    __try
    {
        irp = IoAllocateIrp(1, FALSE);
        if (NULL == irp)
        {
            callbackResult.Result = STATUS_INSUFFICIENT_RESOURCES;
        }
        else
        {
            IoSetCompletionRoutine(irp, IrpComplete, &callbackResult, TRUE, TRUE, TRUE);

            dispatch->WskCloseSocket(Socket, irp);

            KeWaitForSingleObject(&callbackResult.Event, Executive, KernelMode, FALSE, NULL);

            IoFreeIrp(irp);
        }
    }
    __finally
    {
        DbgPrint("Exiting CloseSocket\n");
    }

    return callbackResult.Result;
}

void SetSocketOptions(PIRP Irp, PCALLBACK_RESULT Context)
{
    ULONG optionValue;

    PWSK_PROVIDER_LISTEN_DISPATCH dispatch = (PWSK_PROVIDER_LISTEN_DISPATCH)Global.ListeningSocket->Dispatch;

    optionValue = 0;

    // __debugbreak();

    __try
    {
        if (NULL == Irp)
        {
            DbgPrint("Irp in CreateSocket is NULL\n");
            __leave;
        }

        if (NULL == Context)
        {
            DbgPrint("Callback context in CreateSocket is NULL\n");
            __leave;
        }

        IoSetCompletionRoutine(Irp, IrpComplete, Context, TRUE, TRUE, TRUE);

        dispatch->WskControlSocket(
            Global.ListeningSocket,
            WskSetOption,
            IPV6_V6ONLY,
            IPPROTO_IPV6,
            sizeof(optionValue),
            &optionValue,
            0,
            NULL,
            NULL,
            Irp);

        KeWaitForSingleObject(&Context->Event, Executive, KernelMode, FALSE, NULL);

        IoReuseIrp(Irp, STATUS_UNSUCCESSFUL);
    }
    __finally
    {
        DbgPrint("Exiting SetSocketOptions\n");
    }
}

void BindListeningSocket(PSOCKADDR LocalAddress, PIRP Irp, PCALLBACK_RESULT Context)
{
    PWSK_PROVIDER_LISTEN_DISPATCH dispatch;

    dispatch = (PWSK_PROVIDER_LISTEN_DISPATCH)(Global.ListeningSocket->Dispatch);

    // __debugbreak();

    __try
    {
        if (NULL == LocalAddress)
        {
            DbgPrint("Irp in CreateSocket is NULL\n");
            __leave;
        }

        if (NULL == Irp)
        {
            DbgPrint("Irp in CreateSocket is NULL\n");
            __leave;
        }

        if (NULL == Context)
        {
            DbgPrint("Callback context in CreateSocket is NULL\n");
            __leave;
        }

        IoSetCompletionRoutine(Irp, IrpComplete, Context, TRUE, TRUE, TRUE);

        dispatch->WskBind(Global.ListeningSocket, LocalAddress, 0, Irp);

        KeWaitForSingleObject(&Context->Event, Executive, KernelMode, FALSE, NULL);

        IoReuseIrp(Irp, STATUS_UNSUCCESSFUL);
    }
    __finally
    {
        DbgPrint("Exiting BindListeningSocket\n");
    }
}

void AcceptSocket(PIRP Irp, PCALLBACK_RESULT Context)
{
    PWSK_PROVIDER_LISTEN_DISPATCH dispatch;

    dispatch = (PWSK_PROVIDER_LISTEN_DISPATCH)(Global.ListeningSocket->Dispatch);

    // __debugbreak();

    __try
    {
        if (NULL == Irp)
        {
            DbgPrint("Irp in CreateSocket is NULL\n");
            __leave;
        }

        if (NULL == Context)
        {
            DbgPrint("Callback context in CreateSocket is NULL\n");
            __leave;
        }

        IoSetCompletionRoutine(Irp, AcceptSocketComplete, Context, TRUE, TRUE, TRUE);

        dispatch->WskAccept(Global.ListeningSocket, 0, NULL, NULL, NULL, NULL, Irp);

        KeWaitForSingleObject(&Context->Event, Executive, KernelMode, FALSE, NULL);

        IoReuseIrp(Irp, STATUS_UNSUCCESSFUL);
    }
    __finally
    {
        DbgPrint("Exiting AcceptSocket\n");
    }
}

void InitializeSockets(_In_ PVOID Context)
{
    UNREFERENCED_PARAMETER(Context);

    PIRP irp;

    NTSTATUS result;

    SOCKADDR_IN6 listeningAddress = { AF_INET6, 0x479c, 0, IN6ADDR_ANY_INIT, 0 };

    CALLBACK_RESULT callbackResult = { 0 };

    irp = NULL;

    Global.NpiClient.ClientContext = NULL;
    Global.NpiClient.Dispatch = &WskAppDispatch;
    Global.ListeningSocket = NULL;
    Global.ClientSocket = NULL;
    Global.ProcessProtectionEnabled = 0;
    Global.LocalAddress = (PSOCKADDR)&listeningAddress;

    KeInitializeEvent(&callbackResult.Event, SynchronizationEvent, FALSE);

    __try
    {
        __debugbreak();
        result = WskRegister(&Global.NpiClient, &Global.Registration);
        if (FALSE == NT_SUCCESS(result))
        {
            DbgPrint("Failed to register client\n");
            __leave;
        }

        result = WskCaptureProviderNPI(&Global.Registration, 1000, &Global.Provider);
        if (FALSE == NT_SUCCESS(result))
        {
            DbgPrint("Failed to capture NPI provider\n");
            __leave;
        }

        irp = IoAllocateIrp(1, FALSE);
        if (NULL == irp)
        {
            result = STATUS_INSUFFICIENT_RESOURCES;
            DbgPrint("Failed to allocate irp\n");
            __leave;
        }

        CreateSocket(irp, &callbackResult);
        if (FALSE == NT_SUCCESS(callbackResult.Result))
        {
            DbgPrint("Failed to create socket\n");
            result = callbackResult.Result;
            __leave;
        }

        // __debugbreak();
        WskReleaseProviderNPI(&Global.Registration);

        SetSocketOptions(irp, &callbackResult);
        if (FALSE == NT_SUCCESS(callbackResult.Result))
        {
            DbgPrint("Failed to set socket options\n");
            result = callbackResult.Result;
            __leave;
        }

        BindListeningSocket(Global.LocalAddress, irp, &callbackResult);
        if (FALSE == NT_SUCCESS(callbackResult.Result))
        {
            DbgPrint("Failed to set socket options\n");
            result = callbackResult.Result;
            __leave;
        }

        AcceptSocket(irp, &callbackResult);
        if (FALSE == NT_SUCCESS(callbackResult.Result))
        {
            DbgPrint("Failed to set socket options\n");
            result = callbackResult.Result;
            __leave;
        }

        __debugbreak();
        PROCESS_VALIDATION_RESPONSE response = {0};
        NTSTATUS status = ReceiveData(Global.ClientSocket, &response);
        if (FALSE == NT_SUCCESS(status))
        {
            DbgPrint("ReceiveData failed InitializeSockets\n");
            __leave;
        }

        InitializeProcessProtection(response.ProtectedProcessId);
    }
    __finally
    {
        if (NULL != irp)
        {
            IoFreeIrp(irp);
        }

        // use this method only when running on a separate thread
        PsTerminateSystemThread(STATUS_SUCCESS);
    }
}

NTSTATUS InitializeServer()
{
    NTSTATUS result;

    HANDLE threadHandle;

    threadHandle = NULL;

    __try
    {
        __debugbreak();
        result = PsCreateSystemThread(&threadHandle, THREAD_ALL_ACCESS, NULL, NULL, NULL, (PKSTART_ROUTINE)InitializeSockets, NULL);
        if (FALSE == NT_SUCCESS(result))
        {
            DbgPrint("Failed to create server initialization thread\n");
            __leave;
        }
    }
    __finally
    {
        if (NULL != threadHandle)
        {
            ZwClose(threadHandle);
        }
    }

    return result;
}

void DeinitializeServer()
{
    __debugbreak();

    if (FALSE == NT_SUCCESS(CloseSocket(Global.ListeningSocket)))
    {
        DbgPrint("Failed to close socket\n");
    }
    else
    {
        DbgPrint("SUCCESSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS to close socket\n");
    }

    // WskReleaseProviderNPI(&Global.Registration);
    WskDeregister(&Global.Registration);
}

// Function to send data
NTSTATUS SendData(PWSK_SOCKET Socket, PSEND_CONTEXT Context)
{
    PWSK_PROVIDER_CONNECTION_DISPATCH dispatch;

    WSK_BUF buffer;

    buffer.Offset = 0;
    buffer.Length = Context->Request->RequestLength;
    buffer.Mdl = Context->Mdl;

    // Get pointer to the provider dispatch structure
    dispatch = (PWSK_PROVIDER_CONNECTION_DISPATCH)(Socket->Dispatch);

    IoSetCompletionRoutine(Context->Irp, IrpComplete, &Context->CallBackResult, TRUE, TRUE, TRUE);

    // Initiate the send operation on the socket
    dispatch->WskSend(Socket, &buffer, 0, Context->Irp);

    KeWaitForSingleObject(&Context->CallBackResult.Event, Executive, KernelMode, FALSE, NULL);

    return Context->CallBackResult.Result;
}

NTSTATUS ReceiveData(PWSK_SOCKET Socket, PPROCESS_VALIDATION_RESPONSE ProcessOperationResult)
{
    PWSK_PROVIDER_CONNECTION_DISPATCH dispatch;
    PPROCESS_VALIDATION_RESPONSE response;
    PIRP irp;
    PMDL mdl;

    NTSTATUS result;
    WSK_BUF buffer;
    CALLBACK_RESULT callbackResult;

    // Preinitialization
    irp = NULL;
    mdl = NULL;
    dispatch = NULL;
    response = NULL;
    callbackResult.Result = STATUS_SUCCESS;

    __try
    {
        // Event initialization
        KeInitializeEvent(&callbackResult.Event, SynchronizationEvent, FALSE);

        // Irp allocation
        irp = IoAllocateIrp(1, FALSE);
        if (NULL == irp)
        {
            callbackResult.Result = STATUS_INSUFFICIENT_RESOURCES;
            DbgPrint("Failed to allocate irp\n");
            __leave;
        }

        // Response memory allocation
        response = (PPROCESS_VALIDATION_RESPONSE)ExAllocatePoolWithTag(NonPagedPool, sizeof(*response), RECEIVE_CONTEXT_TAG);
        if (NULL == response)
        {
            callbackResult.Result = STATUS_INSUFFICIENT_RESOURCES;
            DbgPrint("Failed to memory for response irp\n");
            __leave;
        }

        // Allocating MDL
        mdl = IoAllocateMdl(response, sizeof(*response), FALSE, FALSE, NULL);
        if (NULL == mdl)
        {
            result = STATUS_INSUFFICIENT_RESOURCES;
            DbgPrint("Failed to allocate MDL for response\n");
            __leave;
        }

        //Initializing MDL
        MmBuildMdlForNonPagedPool(mdl);

        // WSK buffer initialization
        buffer.Offset = 0;
        buffer.Length = sizeof(*response);
        buffer.Mdl = mdl;

        // Get pointer to the provider dispatch structure
        dispatch = (PWSK_PROVIDER_CONNECTION_DISPATCH)(Socket->Dispatch);

        IoSetCompletionRoutine(irp, IrpComplete, &callbackResult, TRUE, TRUE, TRUE);

        // Initiate the receive operation on the socket
        dispatch->WskReceive(Socket, &buffer, 0, irp);

        KeWaitForSingleObject(&callbackResult.Event, Executive, KernelMode, FALSE, NULL);

        *ProcessOperationResult = *response;
    }
    __finally
    {
        if (NULL != irp)
        {
            IoFreeIrp(irp);
        }

        if (NULL != mdl)
        {
            IoFreeMdl(mdl);
        }

        if (NULL != response)
        {
            ExFreePoolWithTag(response, RECEIVE_CONTEXT_TAG);
        }
    }

    return callbackResult.Result;
}

NTSTATUS InitializeSendContextFromString(PSEND_CONTEXT Context, PUNICODE_STRING ProcessPath, UCHAR ProcessOperationType, HANDLE ProcessId)
{
    NTSTATUS result;

    result = STATUS_SUCCESS;

    __try
    {
        if (NULL == ProcessPath)
        {
            DbgPrint("Process path is null\n");
            result = STATUS_INVALID_PARAMETER;
            __leave;
        }

        Context->Irp = IoAllocateIrp(1, FALSE);
        if (NULL == Context->Irp)
        {
            result = STATUS_INSUFFICIENT_RESOURCES;
            DbgPrint("Failed to allocate irp\n");
            __leave;
        }

        KeInitializeEvent(&Context->CallBackResult.Event, SynchronizationEvent, FALSE);

        Context->Request = (PPROCESS_VALIDATION_REQUEST)ExAllocatePoolWithTag(NonPagedPool, sizeof(*Context->Request) + ProcessPath->Length, SEND_CONTEXT_TAG);
        if (NULL == Context->Request)
        {
            result = STATUS_INSUFFICIENT_RESOURCES;
            DbgPrint("Failed to allocate request structure\n");
            __leave;
        }

        Context->Request->RequestLength = sizeof(*Context->Request) + ProcessPath->Length;
        Context->Request->ProcessOperationType = ProcessOperationType;
        Context->Request->ProcessId = ProcessId;
        RtlCopyMemory(Context->Request->ProcessPath.Buffer, ProcessPath->Buffer, ProcessPath->Length);
        Context->Request->ProcessPath.Length = ProcessPath->Length;

        Context->Mdl = IoAllocateMdl(Context->Request, Context->Request->RequestLength, FALSE, FALSE, NULL);
        if (NULL == Context->Mdl)
        {
            result = STATUS_INSUFFICIENT_RESOURCES;
            DbgPrint("Failed to allocate MDL\n");
            __leave;
        }

        MmBuildMdlForNonPagedPool(Context->Mdl);
    }
    __finally
    {
    }

    return result;
}

NTSTATUS DeinitializeSendContext(PSEND_CONTEXT Context)
{
    NTSTATUS result;

    result = STATUS_SUCCESS;

    __try
    {
        if (NULL == Context)
        {
            DbgPrint("Context is null\n");
            result = STATUS_INVALID_PARAMETER;
            __leave;
        }

        if (NULL != Context->Irp)
        {
            IoFreeIrp(Context->Irp);
            Context->Irp = NULL;
        }

        if (NULL != Context->Mdl)
        {
            IoFreeMdl(Context->Mdl);
            Context->Mdl = NULL;
        }

        ExFreePoolWithTag(Context->Request, SEND_CONTEXT_TAG);
    }
    __finally
    {

    }

    return result;
}
