#pragma once
#include "CIPTask.h"
class CDnsTask : public CIPTask
{
public:
	CDnsTask(IP_Classify* ipClassify, LPCSTR lpszDomain, NTSTATUS ntAccess);
	~CDnsTask();

protected:
	virtual VOID DoTask();
public:
	CHAR m_szDomain[MAX_PATH];
};

