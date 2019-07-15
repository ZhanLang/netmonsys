#pragma once
#include <kutil/string.h>
using namespace msddk;
class CFlowContext:public NonPagedObject
{
public:
	CFlowContext(UINT64 flowHandle, UINT16 layerId, UINT32 calloutId);
	~CFlowContext();

public:
	UINT64 FlowHandle;
	UINT16 LayerId;
	UINT32 CalloutId;
	UINT64 ProcessID;
	CKeStringW ProcessImage;
public:
	static LONG g_FlowCount;
};

