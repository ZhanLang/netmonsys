#include "stdafx.h"
#include "CMetaValue.h"


CMetaValue::CMetaValue(const FWPS_INCOMING_METADATA_VALUES* inMetaValues)
{
	m_inMetaValues = inMetaValues;
	ProcessId = inMetaValues->processId;
}


CMetaValue::~CMetaValue()
{
}

msddk::CKeStringW CMetaValue::GetProcessPath()
{
	CKeStringW Path;
	if ( m_inMetaValues )
	{
		UINT32 uLenght = m_inMetaValues->processPath->size / 2;
		LPWSTR lpBuf = Path.GetBufferSetLength(uLenght);
		memcpy(lpBuf, m_inMetaValues->processPath->data, m_inMetaValues->processPath->size);
		Path.ReleaseBuffer();
	}
	return Path;
}

UINT CMetaValue::GetProcessID()
{
	return (UINT)m_inMetaValues->processId;
}
