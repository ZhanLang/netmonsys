#include "stdafx.h"
#include "CDnsTask.h"
#include "WfpHelp.h"
#include "kfile/path.h"
#include <jansson/json.h>
#include <kutil/strconv.h>


CDnsTask::CDnsTask(IP_Classify* ipClassify, LPCSTR lpszDomain, NTSTATUS ntAccess)
	:CIPTask(ipClassify, ntAccess)
{
	strcpy_s(m_szDomain, _countof(m_szDomain), lpszDomain);
}


CDnsTask::~CDnsTask()
{
}

VOID CDnsTask::DoTask()
{
	WCHAR           szIpAddress[128];
	LPCWSTR          pAddressEnd;
	CKePageStringW		ProcPath;

	ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
	pAddressEnd = Ipv4AddressToString(uRemoteIp, szIpAddress, _countof(szIpAddress));
	CKePath::NtFileNameToDosFileName(ProcessImage, ProcPath);

	json_ptr root = json_object();
	json_ptr value = json_string(u2utf8(ProcPath));
	json_object_set(root, "Process", value);

	value = json_string(u2utf8(szIpAddress));
	json_object_set(root, "RemoteIp", value);

	value = json_string(m_szDomain);
	json_object_set(root, "Domain", value);

	value = json_string(NT_SUCCESS(Action) ? "allow" : "denied");
	json_object_set(root, "Action", value);

	char* data = json_dumps(root, 0);
	SendMessage2(LPC_MESSAGE_ID_DNS_VISIT, data);
	jsonp_free(data);
}
