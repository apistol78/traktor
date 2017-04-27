/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cassert>
#include <sys/synchronization.h>
#include "Core/Thread/Mutex.h"

namespace traktor
{

Mutex::Mutex()
:	m_existing(false)
{
	sys_mutex_attribute_t attr;
	sys_mutex_attribute_initialize(attr);
	attr.attr_recursive = SYS_SYNC_RECURSIVE;
	sys_mutex_create(&m_mutex, &attr);
}

Mutex::Mutex(const Guid& id)
:	m_existing(false)
{
	T_FATAL_ERROR;
}

Mutex::~Mutex()
{
	sys_mutex_destroy(m_mutex);
}

bool Mutex::wait(int32_t timeout)
{
	if (timeout >= 0)
	{
		int rc = sys_mutex_lock(m_mutex, usecond_t(timeout) * 1000);
		return bool(rc == CELL_OK);
	}
	else
	{
		while (sys_mutex_lock(m_mutex, SYS_NO_TIMEOUT) != CELL_OK)
			;
		return true;
	}
}

void Mutex::release()
{
	sys_mutex_unlock(m_mutex);
}

bool Mutex::existing() const
{
	return m_existing;
}

}
