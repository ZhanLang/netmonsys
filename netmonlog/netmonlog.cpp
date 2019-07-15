// netmontest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <windows.h>
#include <atlstr.h>
#include <uframe/frame.h>
#include <netmon_i.h>

#include <json/jsoncpp.hpp>


BOOL SetConsoleColor(WORD wAttributes)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE)
		return FALSE;

	return SetConsoleTextAttribute(hConsole, wAttributes);
}


VOID PrintDns(LPCSTR lpszMsg)
{
	Json::Value Value;
	if (Json::Reader().parse(lpszMsg, Value) )
	{
		std::wstring Process = Value["Process"].asWString();
		std::wstring RemoteIp = Value["RemoteIp"].asWString();
		std::wstring Domain = Value["Domain"].asWString();
		std::wstring Action = Value["Action"].asWString();

		if (Action.compare(L"allow") == 0)
			SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
		else
			SetConsoleColor(FOREGROUND_RED);

		std::wcout << L"DNS过滤:" << L"进程:" << Process << L"\t域名:" << Domain << std::endl;
	}
}

VOID PrintIP(LPCSTR lpszMsg)
{
	Json::Value Value;
	if ( Json::Reader().parse(lpszMsg,Value))
	{
		std::wstring Process = Value["Process"].asWString();
		std::wstring RemoteIp = Value["RemoteIp"].asWString();
		std::wstring Action = Value["Action"].asWString();
		WORD RemotePort = Value["RemotePort"].asInt();

		if (Action.compare(L"allow") == 0)
			SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
		else
			SetConsoleColor(FOREGROUND_RED);

		std::wcout << L"IP过滤:" << L"进程:" << Process << L"\t地址:" << RemoteIp << L":" << RemotePort << std::endl;
	}
}

int _stdcall frame_call(void* param, int uCode, void *pInBuf, int nInCch, void * pOutBuf, int nOutCch, int* nOutSize)
{
	switch (uCode)
	{
	case LPC_MESSAGE_ID_DNS_VISIT:
		PrintDns((LPCSTR)pInBuf);
		break;
	case LPC_MESSAGE_ID_IP_VISIT:
		PrintIP((LPCSTR)pInBuf);
		break;
	default:
		break;
	}
	return 0;
}

int main()
{
	setlocale(LC_ALL, "chs");
	frame_create(NETMONSYS_NAMESPACE, NULL, frame_call);
	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0))
	{

	}
    std::cout << "Hello World!\n"; 
}
