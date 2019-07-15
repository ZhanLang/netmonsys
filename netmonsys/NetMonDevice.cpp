#include "stdafx.h"

#include <kutil\dbg.h>
#include <kutil\sentry.h>
#include <kutil\strconv.h>
#include <kthread\workitem.h>
#include <kthread\basic_thread.h>
#include <ksync\autlock.h>
#include <klog\InOutLog.h>
#include <kfile/path.h>
#include <ktime/time.h>
#include "NetMonDevice.h"
#include <jansson/json.h>

#include "Callouts.h"
#include "CMetaValue.h"
#include "CFlowContext.h"
#include "WfpHelp.h"
#include "windns.h"
#include "CDnsDomainParse.h"
#include "slre.h"
#include "CIPTask.h"
#include "CDnsTask.h"




// {330DA07D-9D6E-4707-B783-93B3965FF6AE}
DEFINE_GUID(MYCALLOUT_ALE_AUTH_CONNECT_V4,
	0x330da07d, 0x9d6e, 0x4707, 0xb7, 0x83, 0x93, 0xb3, 0x96, 0x5f, 0xf6, 0xae);


// {5981CAAF-6E82-4666-9EB6-5A535F340413}
DEFINE_GUID(MYCALLOUT_FLOW_ESTABLISHED_V4,
	0x5981caaf, 0x6e82, 0x4666, 0x9e, 0xb6, 0x5a, 0x53, 0x5f, 0x34, 0x4, 0x13);

// {A3A547E8-9650-4A79-9C75-A9117D8D2D04}
DEFINE_GUID(MYCALLOUT_STREAM_V4,
	0xa3a547e8, 0x9650, 0x4a79, 0x9c, 0x75, 0xa9, 0x11, 0x7d, 0x8d, 0x2d, 0x4);

// {46D56CEA-A4F1-4E50-8010-7568902CA727}
DEFINE_GUID(MYCALLOUT_DATAGRAM_V4,
	0x46d56cea, 0xa4f1, 0x4e50, 0x80, 0x10, 0x75, 0x68, 0x90, 0x2c, 0xa7, 0x27);





CNetMonDevice* CNetMonDevice::g_NetMonDevice = NULL;
CNetMonDevice::CNetMonDevice()
{
	m_hEngine = NULL;
	g_NetMonDevice = this;
	m_bExit = FALSE;
}


CNetMonDevice::~CNetMonDevice()
{
	g_NetMonDevice = NULL;
}

NTSTATUS CNetMonDevice::OnAfterCreate()
{
	NTSTATUS status = STATUS_SUCCESS;
	m_RecordCenter.Start();
	FWPM_SESSION0 Session = { 0 };
	status = FwpmEngineOpen(NULL, RPC_C_AUTHN_WINNT, NULL, &Session, &m_hEngine);
	if ( !NT_SUCCESS(status) )
	{
		KdPrint(("FwpmEngineOpen Failed:%ws\n", MapNTStatus(status)));
		return status;
	}

	status = WfpCalloutRegister();
	if ( !NT_SUCCESS(status) )
	{
		KdPrint(("WfpCalloutRegister Failed:%ws\n", MapNTStatus(status)));
		return status;
	}
	return status;
}

NTSTATUS CNetMonDevice::OnBeforeDelete()
{
	m_bExit = TRUE;
	
	WfpCalloutUnRegister();
	if (m_hEngine != NULL)
	{
		FwpmEngineClose(m_hEngine);
		m_hEngine = NULL;
	}

	m_RecordCenter.Shutdown();
	return STATUS_SUCCESS;
}

LPCWSTR CNetMonDevice::GetDeviceLinkName()
{
	return NETMONSYS_NAMESPACE;
}
LPCWSTR CNetMonDevice::GetDeviceName()
{
	return NETMONSYS_NAMESPACE;
}

NTSTATUS CNetMonDevice::IoCtrlAddWhiteRule(ULONG iocode, LPVOID inBuffer, ULONG inBufferLength, LPVOID outBuffer, ULONG outBufferLenght)
{
	UNREFERENCED_PARAMETER(iocode);
	UNREFERENCED_PARAMETER(inBuffer);
	UNREFERENCED_PARAMETER(inBufferLength);
	UNREFERENCED_PARAMETER(outBuffer);
	UNREFERENCED_PARAMETER(outBufferLenght);


	if ( !(inBuffer && inBufferLength) )
		return STATUS_INVALID_PARAMETER;
	
	const char* json = (const char*)inBuffer;
	
	WHITE_RULE Rule;
	NTSTATUS status = PraseWhiteRule(json, Rule);
	if ( NT_SUCCESS(status) )
	{
		status = InsertWhiteRule(Rule);
	}
	
	if ( !NT_SUCCESS(status ) )
	{
		KdPrint(("IoCtrlAddWhiteRule Faild. %ws\n", MapNTStatus(status)));
	}

	KdPrint(("IoCtrlSetWhiteList\n"));
	return status;
}

NTSTATUS CNetMonDevice::IoCtrlClearWhiteRule(ULONG , LPVOID , ULONG , LPVOID , ULONG )
{
	ClearWhiteRule();
	return STATUS_SUCCESS;
}

NTSTATUS CNetMonDevice::IoCtrlAddIpRule(ULONG iocode, LPVOID inBuffer, ULONG inBufferLength, LPVOID outBuffer, ULONG outBufferLenght)
{
	UNREFERENCED_PARAMETER(iocode);
	UNREFERENCED_PARAMETER(inBuffer);
	UNREFERENCED_PARAMETER(inBufferLength);
	UNREFERENCED_PARAMETER(outBuffer);
	UNREFERENCED_PARAMETER(outBufferLenght);


	if (!(inBuffer && inBufferLength))
		return STATUS_INVALID_PARAMETER;

	const char* json = (const char*)inBuffer;

	IP_PROT_RULE Rule = {0};
	NTSTATUS status = PraseIpRule(json, Rule);
	if (NT_SUCCESS(status))
	{
		status = InsertIpRule(Rule);
	}

	if (!NT_SUCCESS(status))
	{
		KdPrint(("IoCtrlAddIpRule Faild. %ws\n", MapNTStatus(status)));
	}

	KdPrint(("IoCtrlAddIpRule\n"));
	return status;
}

NTSTATUS CNetMonDevice::IoCtrlClearIpRule(ULONG , LPVOID , ULONG , LPVOID , ULONG )
{
	ClearIpRule();
	return STATUS_SUCCESS;
}

NTSTATUS CNetMonDevice::IoCtrlAddDomainRule(ULONG iocode, LPVOID inBuffer, ULONG inBufferLength, LPVOID outBuffer, ULONG outBufferLenght)
{
	UNREFERENCED_PARAMETER(iocode);
	UNREFERENCED_PARAMETER(inBuffer);
	UNREFERENCED_PARAMETER(inBufferLength);
	UNREFERENCED_PARAMETER(outBuffer);
	UNREFERENCED_PARAMETER(outBufferLenght);

	KdPrint(("in IoCtrlAddDomainRule\n"));

	if (!(inBuffer && inBufferLength))
		return STATUS_INVALID_PARAMETER;

	const char* json = (const char*)inBuffer;

	DOMAIN_RULE Rule;
	NTSTATUS status = PraseDomainRule(json, Rule);
	if (NT_SUCCESS(status))
	{
		status = InsertDomainRule(Rule);
	}

	if (!NT_SUCCESS(status))
	{
		KdPrint(("IoCtrlAddDomainRule Faild. %ws\n", MapNTStatus(status)));
	}
	return status;
}

NTSTATUS CNetMonDevice::IoCtrlClearDomainRule(ULONG , LPVOID , ULONG , LPVOID , ULONG )
{
	ClearDomainRule();
	return STATUS_SUCCESS;
}

NTSTATUS CNetMonDevice::WfpCalloutRegister()
{
	NTSTATUS status = STATUS_SUCCESS;
	FlowEstablishedCalloutV4.Initialize(m_pDeviceObject, m_hEngine);
	DatagramCalloutV4.Initialize(m_pDeviceObject, m_hEngine);
	StreamCalloutV4.Initialize(m_pDeviceObject, m_hEngine);
	ALE_AUTH_CONNECT_CALLOUT_V4.Initialize(m_pDeviceObject, m_hEngine);

	//过滤IP和域名
	status = ALE_AUTH_CONNECT_CALLOUT_V4.Register
	(
		L"ALE_AUTH_CONNECT_CALLOUT_V4 Callout",
		L"ALE_AUTH_CONNECT_CALLOUT_V4 Callout Desc",
		&MYCALLOUT_ALE_AUTH_CONNECT_V4,
		&FWPM_LAYER_ALE_AUTH_CONNECT_V4,
		::CalloutClassifyFn,
		::CalloutNotifyFn,
		NULL,
		0
	);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("ALE_AUTH_CONNECT_CALLOUT_V4.Register Failed.%ws", MapNTStatus(status)));
		return status;
	}
	/*
	status = FlowEstablishedCalloutV4.Register
	(
		L"FlowEstablished Callout",
		L"FlowEstablished Callout Desc",
		&MYCALLOUT_FLOW_ESTABLISHED_V4,
		&FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4,
		::CalloutClassifyFn,
		::CalloutNotifyFn,
		NULL,
		0);

	if ( !NT_SUCCESS(status) )
	{
		KdPrint(("FlowEstablishedCalloutV4.Register Failed.%ws", MapNTStatus(status)));
		return status;
	}

	status = StreamCalloutV4.Register
	(
		L"Stream Callout",
		L"Stream Callout Desc",
		&MYCALLOUT_STREAM_V4,
		&FWPM_LAYER_STREAM_V4,
		::CalloutClassifyFn,
		::CalloutNotifyFn,
		::CalloutFlowDeleteFn,
		FWP_CALLOUT_FLAG_CONDITIONAL_ON_FLOW);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("StreamCalloutV4.Register Failed.%ws", MapNTStatus(status)));
		return status;
	}

	status = DatagramCalloutV4.Register
	(
		L"Datagram Callout",
		L"Datagram Callout Desc",
		&MYCALLOUT_DATAGRAM_V4,
		&FWPM_LAYER_DATAGRAM_DATA_V4,
		::CalloutClassifyFn,
		::CalloutNotifyFn,
		::CalloutFlowDeleteFn,
		FWP_CALLOUT_FLAG_CONDITIONAL_ON_FLOW);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("DatagramCalloutV4.Register Failed.%ws", MapNTStatus(status)));
		return status;
	}
	*/

	return status;
}

NTSTATUS CNetMonDevice::WfpCalloutUnRegister()
{
	NTSTATUS status = STATUS_SUCCESS;
	
	{
		CKeFastLocker<CKeSpinLock> AutoLock(m_FlowContextLock);
		CKeMap<UINT64, CFlowContext*, NonPagedObject>::Iterator it = m_FlowContextMap.Min();
		for (; it; it++)
		{
			CFlowContext* Context = (CFlowContext*)it->Value;
			//这个函数会触发CalloutFlowDeleteFn例程，会导致锁重入
			FwpsFlowRemoveContext(Context->FlowHandle, Context->LayerId, Context->CalloutId);
			delete Context;
		}
		m_FlowContextMap.Clear();
	}
	
	ALE_AUTH_CONNECT_CALLOUT_V4.UnRegister();

	/*
	FlowEstablishedCalloutV4.UnRegister();
	DatagramCalloutV4.UnRegister();
	StreamCalloutV4.UnRegister();
	*/
	
	return status;
}

VOID CNetMonDevice::ALE_AUTH_CONNECT_CALLOUT_V4_CLASSIFYFN(
	IN const FWPS_INCOMING_VALUES* inFixedValues, 
	IN const FWPS_INCOMING_METADATA_VALUES* inMetaValues, 
	IN VOID* layerData, 
	IN const void* classifyContext, 
	IN const FWPS_FILTER* filter, 
	IN UINT64 flowContext, 
	OUT FWPS_CLASSIFY_OUT* classifyOut)
{
	UNREFERENCED_PARAMETER(layerData);
	UNREFERENCED_PARAMETER(inMetaValues);
	UNREFERENCED_PARAMETER(classifyContext);
	UNREFERENCED_PARAMETER(flowContext);
	UNREFERENCED_PARAMETER(filter);
	UNREFERENCED_PARAMETER(inFixedValues);
	UNREFERENCED_PARAMETER(classifyOut);

	//如果是内核发出，在等待应用层回调的时候蓝屏。
	/*
	if (inMetaValues->processId == 4)
	{
		KdPrint(("pass system process:%d\n", (UINT)inMetaValues->processId));
		return;
	}
	*/

	NTSTATUS status;
	//栈上的数据，不存在分页
	IP_Classify ipClassify( inFixedValues , inMetaValues);

	status = FilterWhiteRules(&ipClassify);
	if ( NT_SUCCESS(status) )
	{
		return;
	}

	status = FilterIpRules(&ipClassify);
	LogIpConnect(&ipClassify, status);
	if ( !NT_SUCCESS(status) )
	{
		classifyOut->actionType = FWP_ACTION_BLOCK;
		classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
		return;
	}

	//DNS请求
	if (ipClassify.wProtocol == IPPROTO_UDP && ipClassify.wRemotePort == 53)
	{
		PNET_BUFFER_LIST pNetBufferList = (PNET_BUFFER_LIST)layerData;
		PNET_BUFFER pNetBuffer = NET_BUFFER_LIST_FIRST_NB(pNetBufferList);
		UINT32 nDataSize = NET_BUFFER_DATA_LENGTH(pNetBuffer);

		UINT32 dnsLenght = sizeof(DNS_HEADER) + sizeof(UDP_HEADER);
		if (nDataSize >= dnsLenght)
		{
			ke_sentry<PBYTE, ke_default_array_sentry> DnsBuffer = new BYTE[nDataSize];
			PCHAR pInboundData = (PCHAR)NdisGetDataBuffer(pNetBuffer, nDataSize, DnsBuffer, 1, 0);
			if (!pInboundData)
				return;

			pInboundData += dnsLenght;
			USHORT EncodedStrLen = 0;
			ke_sentry<PCHAR, ke_default_array_sentry> Domain = new char[512];
			CDnsDomainParse().DecodeDotStr(pInboundData, &EncodedStrLen, Domain, 512);

			//过滤DNS规则
			status = FilterDnsRules(Domain, &ipClassify);
			LogDnsQuestion(Domain, &ipClassify, status);
			if (!NT_SUCCESS(status))
			{
				classifyOut->actionType = FWP_ACTION_BLOCK;
				classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
			}
		}
	}
}

NTSTATUS CNetMonDevice::FilterDnsRules(LPCSTR szDomain, IP_Classify* ipClassify)
{
	UNREFERENCED_PARAMETER(ipClassify);
	CKeFastLocker<CKeSpinLock> lock(m_DomainRulesLock);
	int len = (int)strlen(szDomain);

	int nSize = m_DomainRules.Size();
	for ( int nLoop = 0 ; nLoop < nSize ; nLoop++)
	{
		DOMAIN_RULE& R = m_DomainRules[nLoop];
		int nMatch = slre_match(R.Domain, szDomain, len, NULL, 0, SLRE_IGNORE_CASE);
		if (nMatch == len)
			return STATUS_ACCESS_DENIED;
	}
	
	return STATUS_SUCCESS;
}

//白名单也使用正则表达式匹配
NTSTATUS CNetMonDevice::FilterIpRules(IP_Classify* ip)
{
	CKeFastLocker<CKeSpinLock> lock(m_IPRulesLock);
	int nSize = m_IPRules.Size();

	for ( int nLoop = 0 ; nLoop <nSize ; nLoop++)
	{
		IP_PROT_RULE rule = m_IPRules[nLoop];

		//只匹配IP地址
		if ( rule.RemoteIp && !rule.RemotePort)
		{
			if ( rule.RemoteIp == ip->uRemoteIp )
				return STATUS_ACCESS_DENIED;
		}

		//只匹配端口
		if ( !rule.RemoteIp && rule.RemotePort )
		{
			if ( rule.RemotePort == ip->wRemotePort )
				return STATUS_ACCESS_DENIED;
		}

		//双重匹配
		if ( rule.RemoteIp && rule.RemotePort)
		{
			if (rule.RemoteIp == ip->uRemoteIp && rule.RemotePort == ip->wRemotePort)
				return STATUS_ACCESS_DENIED;
		}
	}
	return STATUS_SUCCESS;
}


NTSTATUS CNetMonDevice::FilterWhiteRules(IP_Classify* ipClassify)
{
	CKeFastLocker<CKeSpinLock> locker(m_WhiteRulesLock);

	CKeStringW Process = ipClassify->ProcessImage;
	Process.MakeLower();
	INT nRules = m_WhiteRules.Size();
	for ( INT nLoop = 0 ; nLoop < nRules ; nLoop++)
	{
		WHITE_RULE& R = m_WhiteRules[nLoop];
		//不做严格校验了
		if ( Process.Find(R.Process) != -1)
		{
			KdPrint(("FilterWhiteRules Process[%ws] in WhiteRules\n", Process.GetBuffer()));
			return STATUS_SUCCESS;
		}
	}

	return STATUS_NOT_FOUND;
}

VOID CNetMonDevice::LogIpConnect(IP_Classify* ipClassify, NTSTATUS ntAccess)
{
	CIPTask* pTask = new CIPTask(ipClassify,ntAccess);
	m_RecordCenter.EnqueueItem(pTask);
}

VOID CNetMonDevice::LogDnsQuestion(LPCSTR szDomain, IP_Classify* ipClassify, NTSTATUS ntAccess)
{
	CDnsTask* pTask = new CDnsTask( ipClassify, szDomain, ntAccess);
	m_RecordCenter.EnqueueItem(pTask);
}

NTSTATUS CNetMonDevice::InsertWhiteRule(const WHITE_RULE& Rule)
{
	CKeFastLocker<CKeSpinLock> locker(m_WhiteRulesLock);
	INT nSize = m_WhiteRules.Size();
	for ( INT nLoop = 0 ; nLoop < nSize ; nLoop++)
	{
		WHITE_RULE& R = m_WhiteRules[nLoop];
		if ( R.Process.CompareNoCase(Rule.Process) == 0)
			return STATUS_OBJECTID_EXISTS;
	}

	m_WhiteRules.Add(Rule);
	return STATUS_SUCCESS;
}

VOID CNetMonDevice::ClearWhiteRule()
{
	CKeFastLocker<CKeSpinLock> locker(m_WhiteRulesLock);
	m_WhiteRules.Clear();
}

NTSTATUS CNetMonDevice::InsertIpRule(const IP_PROT_RULE& Rule)
{
	CKeFastLocker<CKeSpinLock> locker(m_IPRulesLock);
	INT nSize = m_IPRules.Size();
	for ( INT nLoop = 0 ; nLoop < nSize ; nLoop++)
	{
		IP_PROT_RULE& R = m_IPRules[nLoop];
		if ( R.RemotePort == Rule.RemotePort && R.RemoteIp == Rule.RemoteIp)
			return STATUS_OBJECTID_EXISTS;
	}

	m_IPRules.Add(Rule);
	return STATUS_SUCCESS;
}

VOID CNetMonDevice::ClearIpRule()
{
	CKeFastLocker<CKeSpinLock> locker(m_IPRulesLock);
	m_IPRules.Clear();
}

NTSTATUS CNetMonDevice::InsertDomainRule(const DOMAIN_RULE& Rule)
{
	CKeFastLocker<CKeSpinLock> locker(m_DomainRulesLock);
	INT nSize = m_DomainRules.Size();
	for (INT nLoop = 0; nLoop < nSize; nLoop++)
	{
		DOMAIN_RULE& R = m_DomainRules[nLoop];
		if (R.Domain.CompareNoCase(Rule.Domain) == 0)
			return STATUS_OBJECTID_EXISTS;
	}

	m_DomainRules.Add(Rule);
	return STATUS_SUCCESS;
}

VOID CNetMonDevice::ClearDomainRule()
{
	CKeFastLocker<CKeSpinLock> locker(m_DomainRulesLock);
	m_DomainRules.Clear();
}

NTSTATUS CNetMonDevice::PraseWhiteRule(const char* json, WHITE_RULE& Rule)
{
	if (!json)
		return STATUS_INVALID_PARAMETER;


	json_error_t error;
	json_ptr root = json_loads(json, 0, &error);

	if (!root)
		return STATUS_INVALID_PARAMETER;

	const char* key = NULL;
	json_t* value = NULL;
	json_object_foreach(root, key, value)
	{
		CKePageStringW sPath;
		if (strcmp(key, "Process") == 0)
		{
			const char* proc = json_string_value(value);
			if (!proc)
				return STATUS_INVALID_PARAMETER;

			CKePageStringW sTemp = utf82u<CKePageStringW>(proc);
			NTSTATUS st = CKePath::DosFileNameToNtFileName(sTemp, sPath);
			if (!NT_SUCCESS(st)) //非完全匹配的规则获取完整路径会失败
				sPath = sTemp;

			sPath.MakeLower();
			Rule.Process = sPath.GetBuffer();
		}
	}

	return STATUS_SUCCESS;
}

NTSTATUS CNetMonDevice::PraseIpRule(const char* json, IP_PROT_RULE& Rule)
{
	if (!json)
		return STATUS_INVALID_PARAMETER;


	json_error_t error;
	json_ptr root = json_loads(json, 0, &error);

	if (!root)
		return STATUS_INVALID_PARAMETER;

	const char* key = NULL;
	json_t* value = NULL;
	json_object_foreach(root, key, value)
	{
		if (strcmp(key, "RemoteIp") == 0)
		{
			const char* proc = json_string_value(value);
			if (!proc)
				return STATUS_INVALID_PARAMETER;

			UINT32 IpAddr = 0;
			if ( !NT_SUCCESS(IpV4StringToAddress(proc, IpAddr)) )
				return STATUS_INVALID_PARAMETER;

			Rule.RemoteIp = IpAddr;
		}

		if (strcmp(key, "RemotePort") == 0)
		{
			Rule.RemotePort = (WORD)json_integer_value(value);
		}
	}
	

	//不能全为空
	if (!(Rule.RemoteIp || Rule.RemotePort))
		return STATUS_INVALID_PARAMETER;

	return STATUS_SUCCESS;
}

NTSTATUS CNetMonDevice::PraseDomainRule(const char* json, DOMAIN_RULE& Rule)
{
	if (!json)
		return STATUS_INVALID_PARAMETER;


	json_error_t error;
	json_ptr root = json_loads(json, 0, &error);

	if (!root)
		return STATUS_INVALID_PARAMETER;

	const char* key = NULL;
	json_t* value = NULL;
	json_object_foreach(root, key, value)
	{
		if (strcmp(key, "Domain") == 0)
		{
			const char* proc = json_string_value(value);
			if (!( proc && strlen(proc)) )
				return STATUS_INVALID_PARAMETER;

			Rule.Domain = proc;
			KdPrint(("Rule.Domain:%s\n", Rule.Domain.GetBuffer()));
		}
	}

	return STATUS_SUCCESS;
}

//


NTSTATUS CNetMonDevice::CalloutNotifyFn(
	_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType, 
	_In_ const GUID* filterKey, 
	_Inout_ FWPS_FILTER* filter)
{
	UNREFERENCED_PARAMETER(notifyType);
	UNREFERENCED_PARAMETER(filterKey);
	UNREFERENCED_PARAMETER(filter);

	switch (notifyType)
	{
	case FWPS_CALLOUT_NOTIFY_ADD_FILTER:
		KdPrint(("Filter Added.\n"));
		break;
	case FWPS_CALLOUT_NOTIFY_DELETE_FILTER:
		KdPrint(("Filter Deleted.\n"));
		break;
	}

	return STATUS_SUCCESS;
}



VOID CNetMonDevice::CalloutFlowDeleteFn(IN UINT16 layerId, IN UINT32 calloutId, IN UINT64 flowContext)
{
	UNREFERENCED_PARAMETER(layerId);
	UNREFERENCED_PARAMETER(calloutId);
	UNREFERENCED_PARAMETER(flowContext);

	if ( m_bExit )
	{
		KdPrint(("CalloutFlowDeleteFn->Exit\n"));
		return;
	}
	if ( calloutId == StreamCalloutV4.CalloutID)
	{
		CKeFastLocker<CKeSpinLock> AutoLock(m_FlowContextLock);
		CFlowContext* Context = (CFlowContext*)flowContext;
		FwpsFlowRemoveContext(Context->FlowHandle, layerId, calloutId);
		m_FlowContextMap.Remove(flowContext);
		delete Context;
	}
	if ( calloutId == DatagramCalloutV4.CalloutID)
	{
		//KdPrint(("CalloutFlowDeleteFn->DatagramCalloutV4\n"));
	}
}

VOID CNetMonDevice::CalloutClassifyFn
(
	IN const FWPS_INCOMING_VALUES* inFixedValues, 
	IN const FWPS_INCOMING_METADATA_VALUES* inMetaValues, 
	IN VOID* layerData, 
	IN const void* classifyContext, 
	IN const FWPS_FILTER* filter, 
	IN UINT64 flowContext, 
	OUT FWPS_CLASSIFY_OUT* classifyOut
)
{
	
	if ( m_bExit )
	{
		KdPrint(("CalloutClassifyFn->Exit\n"));
		return;
	}

	UINT32	calloutId = filter->action.calloutId;
	
	if ( calloutId == ALE_AUTH_CONNECT_CALLOUT_V4.CalloutID)
	{
		ALE_AUTH_CONNECT_CALLOUT_V4_CLASSIFYFN(
			inFixedValues,
			inMetaValues,
			layerData,
			classifyContext,
			filter,
			flowContext,
			classifyOut
		);
	}

	classifyOut->actionType = FWP_ACTION_CONTINUE;
	if (calloutId == FlowEstablishedCalloutV4.CalloutID)
	{
		CMetaValue MetaValue(inMetaValues);
		WORD	wProtocol = inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_PROTOCOL].value.uint8;
		if (wProtocol == IPPROTO_TCP || wProtocol == IPPROTO_UDP)
		{
			UINT16 layerID = FWPS_LAYER_STREAM_V4;
			UINT32 callOutID = StreamCalloutV4.CalloutID;

			if (wProtocol == IPPROTO_UDP)
			{
				layerID = FWPS_LAYER_DATAGRAM_DATA_V4;
				callOutID = DatagramCalloutV4.CalloutID;
			}
			
			CFlowContext* Context = new CFlowContext(inMetaValues->flowHandle, layerID, callOutID);
			Context->ProcessID = MetaValue.ProcessId;
			//Context->ProcessImage = MetaValue.GetProcessPath();


			FwpsFlowAssociateContext(inMetaValues->flowHandle, layerID, callOutID, (UINT64)Context);
			{
				CKeFastLocker<CKeSpinLock> AutoLock(m_FlowContextLock);
				m_FlowContextMap.Insert((UINT64)Context, Context);
			}
		}
	}
	if (calloutId == StreamCalloutV4.CalloutID)
	{
		//INT i = (int)flowContext;
	//	
	//	KdPrint(("CalloutClassifyFn->StreamCalloutV4\n"));
	}
	if (calloutId == DatagramCalloutV4.CalloutID)
	{
		KdPrint(("CalloutClassifyFn-> datagram flow:%d\n", flowContext));
		//KdPrint(("CalloutClassifyFn->DatagramCalloutV4\n"));
	}
}

