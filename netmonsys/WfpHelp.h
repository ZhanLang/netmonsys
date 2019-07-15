#pragma once

typedef struct _UDP_HEADER_
{
	UINT16 sourcePort;
	UINT16 destinationPort;
	UINT16 length;
	UINT16 checksum;
}UDP_HEADER, *PUDP_HEADER;

#ifndef s_htons
#define s_htons(num)  ( \
        ( ((USHORT)(num) & 0xff00) >> 8 ) | \
        ( ((USHORT)(num) & 0x00ff) << 8 ) )
#define s_ntohs  s_htons

#endif // !s_htons


#ifndef s_htonl
#define s_htonl(num)  ( \
        ( ((UINT32)(num) & 0xff000000) >> 24 ) | \
        ( ((UINT32)(num) & 0x00ff0000) >> 8 ) | \
        ( ((UINT32)(num) & 0x0000ff00) << 8 ) | \
        ( ((UINT32)(num) & 0x000000ff) << 24 ) )
#define s_ntohl  s_htonl
#endif

LPCWSTR Ipv4AddressToString(UINT32 ipAddr, LPWSTR Str, INT nLen);
NTSTATUS IpV4StringToAddress(LPCWSTR Str, UINT32& Addr);
NTSTATUS IpV4StringToAddress(LPCSTR Str, UINT32& Addr);