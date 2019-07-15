#pragma once

#include <kutil/string.h>
using namespace msddk;
struct IP_Classify: public NonPagedObject
{
public:
	IP_Classify(const FWPS_INCOMING_VALUES* inFixedValues, const FWPS_INCOMING_METADATA_VALUES* inMetaValues);
public:
	WORD wProtocol;	//协议类型
	
	WORD wLocalPort;	//本地端口
	UINT32 uLocalIp;	//本地IP地址

	WORD wRemotePort;	//远程端口
	ULONG uRemoteIp;	//远程IP地址

	INT ProcessID;		//操作者进程
	WCHAR ProcessImage[MAX_PATH];	//进程路径
	
}; 
