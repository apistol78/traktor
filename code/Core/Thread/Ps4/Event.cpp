/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Thread/Event.h"

namespace traktor
{
	namespace
	{

const uint32_t c_broadcast = ~0U;

	}

Event::Event()
:	m_signal(0)
,	m_waiters(0)
{
	ScePthreadMutexattr attr;
	scePthreadMutexattrInit(&attr);
	scePthreadMutexInit(&m_mutex, &attr, 0);

	ScePthreadCondattr attr2;
	scePthreadCondattrInit(&attr2);
	scePthreadCondInit(&m_cond, &attr2, 0);
}

Event::~Event()
{
	scePthreadCondDestroy(&m_cond);
	scePthreadMutexDestroy(&m_mutex);
}

void Event::pulse(int count)
{
	scePthreadMutexLock(&m_mutex);
	
	m_signal += count;
	scePthreadCondBroadcast(&m_cond);

	scePthreadMutexUnlock(&m_mutex);
}

void Event::broadcast()
{
	scePthreadMutexLock(&m_mutex);

	m_signal = c_broadcast;
	scePthreadCondBroadcast(&m_cond);

	scePthreadMutexUnlock(&m_mutex);
}

void Event::reset()
{
	scePthreadMutexLock(&m_mutex);

	m_signal = 0;

	scePthreadMutexUnlock(&m_mutex);
}

bool Event::wait(int32_t timeout)
{
	if (timeout < 0)
		timeout = 0;		// No timeout; blocking until signalled.
	else if (timeout == 0)
		timeout = 1;		// Minuscule timeout.

	int rc = scePthreadMutexLock(&m_mutex);
	if (rc != 0)
		return false;

	if (m_signal == 0)
	{
		m_waiters++;
		while (m_signal == 0)
		{
			rc = scePthreadCondTimedwait(&m_cond, &m_mutex, timeout * 1000);
			if (rc != 0)
				break;
		}
		m_waiters--;
	}

	if (rc == 0 && m_signal != 0)
	{
		if (m_signal != c_broadcast)
			m_signal--;
		else if (m_waiters == 0)
			m_signal = 0;
	}

	scePthreadMutexUnlock(&m_mutex);
	
	return bool(rc == 0);
}

}
