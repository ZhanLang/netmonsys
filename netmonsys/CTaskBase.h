#pragma once
#include <kutil/memory.h>
#include <netmon_i.h>
using namespace msddk;
class CTaskBase : public NonPagedObject
{
public:
	CTaskBase() {}
	virtual ~CTaskBase() {}

	NTSTATUS SendMessage2(int msg, const char* data);

public:
	virtual VOID DoTask() = 0;
};
