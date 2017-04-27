/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <sys/synchronization.h>
#include "Core/Thread/Signal.h"

namespace traktor
{
	namespace
	{
	
struct Internal
{
	sys_mutex_t mutex;
	sys_cond_t cond;
	uint32_t signal;
};
	
	}

Signal::Signal()
:	m_handle(0)
{
	Internal* in = new Internal();

	sys_mutex_attribute_t attr1;
	sys_mutex_attribute_initialize(attr1);
	sys_mutex_create(&in->mutex, &attr1);

	sys_cond_attribute_t attr2;
	sys_cond_attribute_initialize(attr2);
	sys_cond_create(&in->cond, in->mutex, &attr2);

	in->signal = 0;

	m_handle = in;
}

Signal::~Signal()
{
	Internal* in = static_cast< Internal* >(m_handle);
	T_ASSERT (in);

	sys_cond_destroy(in->cond);
	sys_mutex_destroy(in->mutex);

	delete in;
}

void Signal::set()
{
	Internal* in = static_cast< Internal* >(m_handle);
	T_ASSERT (in);

	sys_mutex_lock(in->mutex, SYS_NO_TIMEOUT);
	
	in->signal = 1;
	sys_cond_signal_all(in->cond);

	sys_mutex_unlock(in->mutex);
}

void Signal::reset()
{
	Internal* in = static_cast< Internal* >(m_handle);
	T_ASSERT (in);

	sys_mutex_lock(in->mutex, SYS_NO_TIMEOUT);
	
	in->signal = 0;

	sys_mutex_unlock(in->mutex);
}

bool Signal::wait(int32_t timeout)
{
	Internal* in = static_cast< Internal* >(m_handle);
	T_ASSERT (in);

	int rc = sys_mutex_lock(in->mutex, SYS_NO_TIMEOUT);
	if (rc != CELL_OK)
		return false;

	while (in->signal == 0)
	{
		rc = sys_cond_wait(in->cond, usecond_t(timeout) * 1000);
		if (rc == ETIMEDOUT)
			break;
	}

	sys_mutex_unlock(in->mutex);
	
	return bool(rc == CELL_OK);
}

}
