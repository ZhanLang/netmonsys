#include "stdafx.h"
#include <stdio.h>
#include "WfpHelp.h"
#include <kutil/string.h>
#include <Mstcpip.h>
LPCWSTR Ipv4AddressToString(UINT32 ipAddr, LPWSTR Str, INT nLen)
{
	BYTE *pbyIPSegment = (BYTE*)(&ipAddr);
	_snwprintf(Str, nLen, L"%d.%d.%d.%d", pbyIPSegment[0], pbyIPSegment[1], pbyIPSegment[2], pbyIPSegment[3]);
	return Str;
}

NTSTATUS IpV4StringToAddress(LPCWSTR Str, UINT32& Addr)
{
	LPCWSTR lpEnd;
	IN_ADDR IpAddr;
	NTSTATUS status = RtlIpv4StringToAddressW(Str, FALSE, &lpEnd, &IpAddr);
	if ( NT_SUCCESS(status) )
		Addr = IpAddr.S_un.S_addr;
	
	return status;
}

NTSTATUS IpV4StringToAddress(LPCSTR Str, UINT32& Addr)
{
	LPCSTR lpEnd;
	IN_ADDR IpAddr;
	NTSTATUS status = RtlIpv4StringToAddressA(Str, FALSE, &lpEnd, &IpAddr);
	if (NT_SUCCESS(status))
		Addr = IpAddr.S_un.S_addr;

	return status;
}
