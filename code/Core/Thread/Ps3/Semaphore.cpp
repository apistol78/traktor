/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Thread/Semaphore.h"
#include "Core/Misc/TString.h"

namespace traktor
{

Semaphore::Semaphore()
{
	sys_lwmutex_attribute_t attr;
	sys_lwmutex_attribute_initialize(attr);
	attr.attr_recursive = SYS_SYNC_RECURSIVE;
	sys_lwmutex_create(&m_mutex, &attr);
}

Semaphore::~Semaphore()
{
	sys_lwmutex_destroy(&m_mutex);
}

bool Semaphore::wait(int32_t timeout)
{
	if (timeout >= 0)
	{
		int rc = sys_lwmutex_lock(&m_mutex, usecond_t(timeout) * 1000);
		return bool(rc == CELL_OK);
	}
	else
	{
		while (sys_lwmutex_lock(&m_mutex, SYS_NO_TIMEOUT) != CELL_OK)
			;
		return true;
	}
}

void Semaphore::release()
{
	sys_lwmutex_unlock(&m_mutex);
}

}
