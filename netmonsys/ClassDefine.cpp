#include "stdafx.h"
#include "ClassDefine.h"
#include "WfpHelp.h"
#include "CMetaValue.h"

IP_Classify::IP_Classify(const FWPS_INCOMING_VALUES* inFixedValues,const FWPS_INCOMING_METADATA_VALUES* inMetaValues)
{
	UINT32 nIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_PORT;
	wLocalPort = inFixedValues->incomingValue[nIndex].value.int16;

	nIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_ADDRESS;
	uLocalIp = s_htonl(inFixedValues->incomingValue[nIndex].value.int32);

	nIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_PORT;
	wRemotePort = inFixedValues->incomingValue[nIndex].value.int16;

	nIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_ADDRESS;
	uRemoteIp = s_htonl(inFixedValues->incomingValue[nIndex].value.int32);

	nIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_PROTOCOL;
	wProtocol = inFixedValues->incomingValue[nIndex].value.uint8;

	memcpy_s(ProcessImage, sizeof(ProcessImage), inMetaValues->processPath->data, inMetaValues->processPath->size);
	ProcessImage[_countof(ProcessImage) - 1] = 0;
	ProcessID = (UINT)inMetaValues->processId;
}
