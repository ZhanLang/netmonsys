// netmonset.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <windows.h>
#include <uframe/frame.h>
#include <json/jsoncpp.hpp>
#include <util/cconv.h>
#include <netmon_i.h>
using namespace msdk;
HRESULT InsertWhiteRule(LPCWSTR lpszProcess)
{
	Json::Value value;
	value["Process"] = w2utf8(lpszProcess);

	std::string json = Json::FastWriter().write(value);
	HRESULT hResult = frame_send2(NETMONSYS_NAMESPACE, IOCT_ADD_WHITE_RULE, (LPVOID)json.c_str(), json.length() + 1, 0, 0, 0);
	if ( SUCCEEDED(hResult) )
	{
		std::wcout << L"设置白名单策略成功" << lpszProcess << std::endl;
	}
	return hResult;
}

HRESULT InsertIPRule(LPCWSTR lpszIP, WORD Port)
{
	Json::Value value;
	value["RemoteIp"] = lpszIP;
	value["RemotePort"] = Port;

	std::string json = Json::FastWriter().write(value);
	HRESULT hResult = frame_send2(NETMONSYS_NAMESPACE, IOCT_ADD_IP_RULE, (LPVOID)json.c_str(), json.length() + 1, 0, 0, 0);
	if (SUCCEEDED(hResult))
	{
		std::wcout << L"设置IP策略成功" << lpszIP << std::endl;
	}
	return hResult;
}


HRESULT InsertDomainRule(LPCSTR lpszDomain)
{
	Json::Value value;
	value["Domain"] = lpszDomain;
	

	std::string json = Json::FastWriter().write(value);
	HRESULT hResult = frame_send2(NETMONSYS_NAMESPACE, IOCT_ADD_DOMAIN_RULE, (LPVOID)json.c_str(), json.length() + 1, 0, 0, 0);
	if (SUCCEEDED(hResult))
	{
		std::cout << L"Domain" << lpszDomain << std::endl;
	}

	return hResult;
}

int main()
{
	setlocale(LC_ALL, "chs");

	frame_send2(NETMONSYS_NAMESPACE, IOCT_CLEAR_WHITE_RULE, 0, 0, 0, 0, 0);
	frame_send2(NETMONSYS_NAMESPACE, IOCT_CLEAR_IP_RULE, 0, 0, 0, 0, 0);
	frame_send2(NETMONSYS_NAMESPACE, IOCT_CLEAR_DOMAIN_RULE, 0, 0, 0, 0, 0);

	InsertWhiteRule(L"iexplore.exe");
	InsertIPRule(L"220.181.111.37", 443);
	InsertIPRule(L"220.181.57.216", 443);
	InsertIPRule(L"69.171.224.12", 443);
	InsertIPRule(L"216.58.197.110", 443);
	InsertIPRule(L"220.181.112.244", 443);	//百度的网址
	InsertDomainRule(".*.hao123.com");
	InsertDomainRule(".*.2345.com");
    std::cout << "Hello World!\n"; 
}