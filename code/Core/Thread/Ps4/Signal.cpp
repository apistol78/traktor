/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <kernel.h>
#include "Core/Thread/Signal.h"

namespace traktor
{
	namespace
	{
	
struct Internal
{
	ScePthreadMutex mutex;
	ScePthreadCond cond;
	uint32_t signal;
};
	
	}

Signal::Signal()
:	m_handle(0)
{
	Internal* in = new Internal();

	ScePthreadMutexattr attr;
	scePthreadMutexattrInit(&attr);
	scePthreadMutexInit(&in->mutex, &attr, 0);

	ScePthreadCondattr attr2;
	scePthreadCondattrInit(&attr2);
	scePthreadCondInit(&in->cond, &attr2, 0);

	in->signal = 0;

	m_handle = in;
}

Signal::~Signal()
{
	Internal* in = static_cast< Internal* >(m_handle);
	T_ASSERT (in);

	scePthreadCondDestroy(&in->cond);
	scePthreadMutexDestroy(&in->mutex);

	delete in;
}

void Signal::set()
{
	Internal* in = static_cast< Internal* >(m_handle);
	T_ASSERT (in);

	scePthreadMutexLock(&in->mutex);
	
	in->signal = 1;
	scePthreadCondBroadcast(&in->cond);

	scePthreadMutexUnlock(&in->mutex);
}

void Signal::reset()
{
	Internal* in = static_cast< Internal* >(m_handle);
	T_ASSERT (in);

	scePthreadMutexLock(&in->mutex);
	
	in->signal = 0;

	scePthreadMutexUnlock(&in->mutex);
}

bool Signal::wait(int32_t timeout)
{
	Internal* in = static_cast< Internal* >(m_handle);
	T_ASSERT (in);

	int rc = scePthreadMutexLock(&in->mutex);
	if (rc != 0)
		return false;

	while (in->signal == 0)
	{
		rc = scePthreadCondTimedwait(&in->cond, &in->mutex, timeout * 1000);
		if (rc == SCE_KERNEL_ERROR_ETIMEDOUT)
			break;
	}

	scePthreadMutexUnlock(&in->mutex);
	
	return bool(rc == 0);
}

}
