#pragma once
class CDnsDomainParse
{
public:
	CDnsDomainParse();
	~CDnsDomainParse();

	NTSTATUS DecodeDotStr(char *szEncodedStr, USHORT *pusEncodedStrLen, char *szDotStr, USHORT nDotStrSize, char *szPacketStartPos = NULL);
};

