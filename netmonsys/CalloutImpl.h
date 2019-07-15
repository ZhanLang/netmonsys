#pragma once

#include <fwpsk.h>
class CalloutImpl
{
public:
	CalloutImpl();
	~CalloutImpl();

	VOID Initialize(PDEVICE_OBJECT obj, HANDLE hEngine);

	NTSTATUS Register(IN PWCHAR wszName,
		IN PWCHAR wszDesc,
		IN const GUID* calloutKey,
		IN const GUID* layerKey,
		IN FWPS_CALLOUT_CLASSIFY_FN classifyFn,
		IN  FWPS_CALLOUT_NOTIFY_FN notifyFn,
		IN  FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN flowDeleteNotifyFn,
		IN UINT32 flags);

	NTSTATUS UnRegister();

public:
	UINT32 CalloutID;
	UINT32 fwpmId;
	UINT64 filterId;
private:
	HANDLE m_hEngine;
	PDEVICE_OBJECT m_DeviceObject;
	
};

