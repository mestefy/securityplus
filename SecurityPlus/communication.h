#pragma once

#include <ntddk.h>
#include <wsk.h>

#define PROCESS_CREATE 0
#define PROCESS_CLOSE 1

#define PROCESS_OPERATION_ALLOW 1
#define PROCESS_OPERATION_DENY 0

#define SEND_CONTEXT_TAG 'DNS'
#define RECEIVE_CONTEXT_TAG 'VCR'

#pragma pack(1)
typedef struct _STRING_DATA
{
    USHORT Length;
    UCHAR Buffer[0];
}STRING_DATA, *PSTRING_DATA;

typedef struct _PROCESS_VALIDATION_REQUEST
{
    LONG RequestLength;
    UCHAR ProcessOperationType;
    HANDLE ProcessId;
    STRING_DATA ProcessPath;
    // UNICODE_STRING ProcessPath;
    // UCHAR Buffer[0];
}PROCESS_VALIDATION_REQUEST, *PPROCESS_VALIDATION_REQUEST;

typedef struct _PROCESS_VALIDATION_RESPONSE
{
    // union
    // {
        UCHAR ProcessOperationResult;
        HANDLE ProtectedProcessId;
    // };
}PROCESS_VALIDATION_RESPONSE, *PPROCESS_VALIDATION_RESPONSE;
#pragma pack()

typedef struct _WSK_GLOBAL
{
    WSK_CLIENT_NPI NpiClient;
    WSK_REGISTRATION Registration;
    WSK_PROVIDER_NPI Provider;
    PWSK_SOCKET ListeningSocket;
    PWSK_SOCKET ClientSocket;
    WSK_CLIENT_CONNECTION_DISPATCH SocketDispatch;
    PSOCKADDR LocalAddress;
    int ProcessProtectionEnabled;
}WSK_GLOBAL, *PWSK_GLOBAL;

WSK_GLOBAL Global;

typedef struct _CALLBACK_RESULT
{
    NTSTATUS Result;
    KEVENT Event;
}CALLBACK_RESULT, *PCALLBACK_RESULT;

typedef struct _SEND_CONTEXT
{
    PIRP Irp;
    PMDL Mdl;
    CALLBACK_RESULT CallBackResult;
    PPROCESS_VALIDATION_REQUEST Request;
}SEND_CONTEXT, *PSEND_CONTEXT;

typedef struct _RECEIVE_CONTEXT
{
    PIRP Irp;
    PMDL Mdl;
    CALLBACK_RESULT CallBackResult;
    PPROCESS_VALIDATION_RESPONSE Response;
}RECEIVE_CONTEXT, *PRECEIVE_CONTEXT;

NTSTATUS InitializeServer();

void DeinitializeServer();

NTSTATUS SendData(PWSK_SOCKET Socket, PSEND_CONTEXT Context);

NTSTATUS ReceiveData(PWSK_SOCKET Socket, PPROCESS_VALIDATION_RESPONSE ProcessOperationResult);

NTSTATUS CloseSocket(PWSK_SOCKET Socket);

NTSTATUS InitializeSendContextFromString(PSEND_CONTEXT Context, PUNICODE_STRING ProcessPath, UCHAR ProcessOperationType, HANDLE ProcessId);

NTSTATUS DeinitializeSendContext(PSEND_CONTEXT Context);
