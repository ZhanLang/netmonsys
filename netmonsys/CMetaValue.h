#pragma once
#include <kutil/string.h>
using namespace msddk;
class CMetaValue
{
public:
	CMetaValue(const FWPS_INCOMING_METADATA_VALUES* inMetaValues);
	~CMetaValue();

	CKeStringW GetProcessPath();
	UINT GetProcessID();
public:
	UINT64 ProcessId;
private:
	const FWPS_INCOMING_METADATA_VALUES* m_inMetaValues;
};

