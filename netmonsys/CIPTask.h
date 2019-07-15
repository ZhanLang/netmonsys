#pragma once
#include "CTaskBase.h"
#include "ClassDefine.h"
class CIPTask :
	public CTaskBase
{
public:
	CIPTask(IP_Classify* ipClassify, NTSTATUS ntAccess);
	~CIPTask();

	virtual VOID DoTask();

protected:
	WORD wProtocol;		//协议类型
	DWORD wLocalPort;	//本地端口
	UINT32 uLocalIp;	//本地IP地址
	DWORD wRemotePort;	//远程端口
	ULONG uRemoteIp;	//远程IP地址
	INT ProcessID;		//操作者进程
	NTSTATUS	Action;			//授权结果
	WCHAR ProcessImage[MAX_PATH];	//进程路径
};

