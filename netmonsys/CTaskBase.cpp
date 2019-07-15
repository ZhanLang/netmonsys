#pragma once
#include "stdafx.h"
#include "CTaskBase.h"
#include <klpc/client.h>

NTSTATUS CTaskBase::SendMessage2(int msg, const char* data)
{
	return CKeLpcClient(NETMONSYS_NAMESPACE).SendMessage(msg, (LPVOID)data, (int)strlen(data) + 1, NULL, 0, NULL, NULL);
}

