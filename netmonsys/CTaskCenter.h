#pragma once

#include <kthread/workitem.h>
#include "CTaskBase.h"
#include <kthread/worker_thread.h>
using namespace msddk;
class CTaskCenter : public CkeWorkerThread
{
public:
	CTaskCenter() {}
	~CTaskCenter(){}
	
protected:
	virtual void DispatchItem(void *pItem)
	{
		CTaskBase* pTask = (CTaskBase*)pItem;
		if (pTask)
		{
			pTask->DoTask();
			delete pTask;
		}
	}
};

