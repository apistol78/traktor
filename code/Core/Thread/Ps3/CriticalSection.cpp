/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <sys/synchronization.h>
#include "Core/Thread/CriticalSection.h"

namespace traktor
{

CriticalSection::CriticalSection()
{
	sys_lwmutex_attribute_t attr;
	sys_lwmutex_attribute_initialize(attr);
	sys_lwmutex_create(&m_mutex, &attr);
}

CriticalSection::~CriticalSection()
{
	sys_lwmutex_destroy(&m_mutex);
}

bool CriticalSection::wait(int32_t timeout)
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

void CriticalSection::release()
{
	sys_lwmutex_unlock(&m_mutex);
}

}

