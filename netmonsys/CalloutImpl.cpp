#include "stdafx.h"
#include "CalloutImpl.h"
#include "kutil\dbg.h"


using namespace msddk;
CalloutImpl::CalloutImpl()
{
	CalloutID = 0;
	fwpmId = 0;
	filterId = 0;
}

CalloutImpl::~CalloutImpl()
{
}

VOID CalloutImpl::Initialize(PDEVICE_OBJECT obj, HANDLE hEngine)
{
	m_DeviceObject = obj;
	m_hEngine = hEngine;
}

NTSTATUS CalloutImpl::Register(IN PWCHAR wszName, IN PWCHAR wszDesc, IN const GUID* calloutKey, IN const GUID* layerKey, IN FWPS_CALLOUT_CLASSIFY_FN classifyFn, IN FWPS_CALLOUT_NOTIFY_FN notifyFn, IN FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN flowDeleteNotifyFn, IN UINT32 flags)
{
	NTSTATUS        status = STATUS_SUCCESS;
	FWPS_CALLOUT	sCallout = { 0 };
	FWPM_CALLOUT	mCallout = { 0 };
	FWPM_FILTER		mFilter = { 0 };

	sCallout.calloutKey = *calloutKey;
	sCallout.flags = flags;
	sCallout.classifyFn = classifyFn;
	sCallout.flowDeleteFn = flowDeleteNotifyFn;
	sCallout.notifyFn = notifyFn;
	status = FwpsCalloutRegister(m_DeviceObject, &sCallout, &CalloutID);
	if ( !NT_SUCCESS(status) )
	{
		KdPrint(("FwpsCalloutRegister(%ws,%ws) faild.%ws", wszName, wszDesc, MapNTStatus(status)));
		return status;
	}

	mCallout.displayData.name		 = wszName;
	mCallout.displayData.description = wszDesc;
	mCallout.calloutKey				 = *calloutKey;
	mCallout.applicableLayer		 = *layerKey;
	status = FwpmCalloutAdd(m_hEngine, &mCallout, NULL, &fwpmId);
	if ( !NT_SUCCESS( status) )
	{
		KdPrint(("FwpmCalloutAdd(%ws,%ws) faild.%ws", wszName, wszDesc, MapNTStatus(status)));
		return status;
	}



	mFilter.action.calloutKey = *calloutKey;
	mFilter.action.type = FWP_ACTION_CALLOUT_TERMINATING;
	mFilter.displayData.name = wszName;
	mFilter.displayData.description = wszDesc;
	mFilter.layerKey = *layerKey;
	mFilter.numFilterConditions = 0;
	mFilter.filterCondition = NULL;
	mFilter.subLayerKey = FWPM_SUBLAYER_UNIVERSAL;
	mFilter.weight.type = FWP_EMPTY;
	status = FwpmFilterAdd(m_hEngine, &mFilter, NULL, &filterId);
	if ( !NT_SUCCESS(status) )
	{
		KdPrint(("FwpmFilterAdd(%ws,%ws) faild.%ws", wszName, wszDesc, MapNTStatus(status)));
		return status;
	}

	return status;
}

NTSTATUS CalloutImpl::UnRegister()
{
	if (filterId)
	{
		FwpmFilterDeleteById(m_hEngine, filterId);
		filterId = 0;
	}

	if (fwpmId)
	{
		FwpmCalloutDeleteById(m_hEngine, fwpmId);
		fwpmId = 0;
	}

	if (CalloutID)
	{
		FwpsCalloutUnregisterById(CalloutID);
		CalloutID = 0;
	}

	return STATUS_SUCCESS;
}
