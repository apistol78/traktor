/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Thread/Mutex.h"

namespace traktor
{

Mutex::Mutex()
:	m_existing(false)
{
	ScePthreadMutexattr attr;
	scePthreadMutexattrInit(&attr);
	scePthreadMutexattrSettype(&attr, SCE_PTHREAD_MUTEX_RECURSIVE);
	scePthreadMutexInit(&m_mutex, &attr, 0);
}

Mutex::Mutex(const Guid& id)
:	m_existing(false)
{
	T_FATAL_ERROR;
}

Mutex::~Mutex()
{
	scePthreadMutexDestroy(&m_mutex);
}

bool Mutex::wait(int32_t timeout)
{
	if (timeout >= 0)
		return scePthreadMutexTimedlock(&m_mutex, timeout * 1000) == 0;
	else
		return scePthreadMutexLock(&m_mutex) == 0;
}

void Mutex::release()
{
	scePthreadMutexUnlock(&m_mutex);
}

bool Mutex::existing() const
{
	return m_existing;
}

}
