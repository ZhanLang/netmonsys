#include "stdafx.h"
#include "CFlowContext.h"


LONG CFlowContext::g_FlowCount = 0;


CFlowContext::CFlowContext(UINT64 flowHandle, UINT16 layerId, UINT32 calloutId)
{
	FlowHandle = flowHandle;
	LayerId = layerId;
	CalloutId = calloutId;
	ProcessID = 0;
	ULONG count = InterlockedIncrement(&g_FlowCount); (count);
	KdPrint(("FlowContext->g_FlowCount:%d\n", count));
}

CFlowContext::~CFlowContext()
{
	ULONG count = InterlockedDecrement(&g_FlowCount); (count);
	KdPrint(("FlowContext->g_FlowCount:%d\n", count));
}
