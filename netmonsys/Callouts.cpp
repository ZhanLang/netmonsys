#include "stdafx.h"
#include "Callouts.h"
#include "NetMonDevice.h"
NTSTATUS CalloutNotifyFn(
	_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	_In_ const GUID* filterKey,
	_Inout_ FWPS_FILTER* filter)
{
	return CNetMonDevice::g_NetMonDevice->CalloutNotifyFn
	(
		notifyType, 
		filterKey, 
		filter
	);
}

VOID CalloutFlowDeleteFn(IN UINT16 layerId, IN UINT32 calloutId, IN UINT64 flowContext)
{
	CNetMonDevice::g_NetMonDevice->CalloutFlowDeleteFn
	(
		layerId, 
		calloutId, 
		flowContext
	);
}

VOID CalloutClassifyFn(
	IN const FWPS_INCOMING_VALUES* inFixedValues, 
	IN const FWPS_INCOMING_METADATA_VALUES* inMetaValues, 
	IN VOID* layerData, 
	IN const void* classifyContext, 
	IN const FWPS_FILTER* filter, 
	IN UINT64 flowContext, 
	OUT FWPS_CLASSIFY_OUT* classifyOut)
{
	CNetMonDevice::g_NetMonDevice->CalloutClassifyFn
	(
		inFixedValues, 
		inMetaValues, 
		layerData, 
		classifyContext, 
		filter, 
		flowContext, 
		classifyOut
	);
}