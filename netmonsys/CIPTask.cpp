#include "stdafx.h"
#include <kfile/path.h>
#include "CIPTask.h"
#include <Mstcpip.h>
#include "WfpHelp.h"
#include <jansson/json.h>
#include <kutil/strconv.h>

CIPTask::CIPTask(IP_Classify* ipClassify, NTSTATUS ntAccess)
{
	wProtocol = ipClassify->wProtocol;
	wLocalPort = ipClassify->wLocalPort;
	uLocalIp = ipClassify->uLocalIp;
	wRemotePort = ipClassify->wRemotePort;
	uRemoteIp = ipClassify->uRemoteIp;
	ProcessID = ipClassify->ProcessID;
	Action = ntAccess;
	memcpy(ProcessImage, ipClassify->ProcessImage, sizeof(ProcessImage));
}



CIPTask::~CIPTask()
{
}

VOID CIPTask::DoTask()
{
	WCHAR           szIpAddress[128];
	LPCWSTR          pAddressEnd;
	CKePageStringW		ProcPath;

	ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

	pAddressEnd = Ipv4AddressToString(uRemoteIp,szIpAddress,_countof(szIpAddress));
	CKePath::NtFileNameToDosFileName(ProcessImage, ProcPath);

	//直接使用json上报

	json_ptr root = json_object();
	json_ptr value = json_string(u2utf8(ProcPath));
	json_object_set(root, "Process", value);

	value = json_string(u2utf8(szIpAddress));
	json_object_set(root, "RemoteIp", value);
	

	value = json_integer(wRemotePort);
	json_object_set(root, "RemotePort" ,value);

	value = json_string(NT_SUCCESS(Action)?"allow" : "denied" );
	json_object_set(root, "Action", value);


	char* data = json_dumps(root, 0);
	SendMessage2(LPC_MESSAGE_ID_IP_VISIT, data);
	jsonp_free(data);
}
