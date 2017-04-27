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

const uint32_t c_broadcast = ~0UL;

	}

Event::Event()
:	m_signal(0)
,	m_waiters(0)
{
	sys_lwmutex_attribute_t attr1;
	sys_lwmutex_attribute_initialize(attr1);
	sys_lwmutex_create(&m_mutex, &attr1);

	sys_lwcond_attribute_t attr2;
	sys_lwcond_attribute_initialize(attr2);
	sys_lwcond_create(&m_cond, &m_mutex, &attr2);
}

Event::~Event()
{
	sys_lwcond_destroy(&m_cond);
	sys_lwmutex_destroy(&m_mutex);
}

void Event::pulse(int count)
{
	sys_lwmutex_lock(&m_mutex, SYS_NO_TIMEOUT);
	
	m_signal += count;
	sys_lwcond_signal_all(&m_cond);

	sys_lwmutex_unlock(&m_mutex);
}

void Event::broadcast()
{
	sys_lwmutex_lock(&m_mutex, SYS_NO_TIMEOUT);

	m_signal = c_broadcast;
	sys_lwcond_signal_all(&m_cond);

	sys_lwmutex_unlock(&m_mutex);
}

void Event::reset()
{
	sys_lwmutex_lock(&m_mutex, SYS_NO_TIMEOUT);

	m_signal = 0;

	sys_lwmutex_unlock(&m_mutex);
}

bool Event::wait(int32_t timeout)
{
	if (timeout < 0)
		timeout = 0;		// No timeout; blocking until signalled.
	else if (timeout == 0)
		timeout = 1;		// Minuscule timeout.

	int rc = sys_lwmutex_lock(&m_mutex, SYS_NO_TIMEOUT);
	if (rc != CELL_OK)
		return false;

	if (m_signal == 0)
	{
		m_waiters++;
		while (m_signal == 0)
		{
			rc = sys_lwcond_wait(&m_cond, usecond_t(timeout) * 1000);
			if (rc != CELL_OK)
				break;
		}
		m_waiters--;
	}

	if (rc == CELL_OK && m_signal != 0)
	{
		if (m_signal != c_broadcast)
			m_signal--;
		else if (m_waiters == 0)
			m_signal = 0;
	}

	sys_lwmutex_unlock(&m_mutex);
	
	return bool(rc == CELL_OK);
}

}
