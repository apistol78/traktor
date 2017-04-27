/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Thread/Semaphore.h"

namespace traktor
{

Semaphore::Semaphore()
{
	ScePthreadMutexattr attr;
	scePthreadMutexattrInit(&attr);
	scePthreadMutexattrSettype(&attr, SCE_PTHREAD_MUTEX_RECURSIVE);
	scePthreadMutexInit(&m_mutex, &attr, 0);
}

Semaphore::~Semaphore()
{
	scePthreadMutexDestroy(&m_mutex);
}

bool Semaphore::wait(int32_t timeout)
{
	if (timeout >= 0)
		return scePthreadMutexTimedlock(&m_mutex, timeout * 1000) == 0;
	else
		return scePthreadMutexLock(&m_mutex) == 0;
}

void Semaphore::release()
{
	scePthreadMutexUnlock(&m_mutex);
}

}
