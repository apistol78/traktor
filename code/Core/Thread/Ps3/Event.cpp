#include <cassert>
#include <sys/synchronization.h>
#include "Core/Thread/Event.h"

namespace traktor
{
	namespace
	{
	
struct Internal
{
	sys_mutex_t mutex;
	sys_cond_t cond;
};
	
	}

Event::Event()
:	m_handle(0)
{
	Internal* in = new Internal();

	sys_mutex_attribute_t attr1;
	sys_mutex_attribute_initialize(attr1);
	sys_mutex_create(&in->mutex, &attr1);

	sys_cond_attribute_t attr2;
	sys_cond_attribute_initialize(attr2);
	sys_cond_create(&in->cond, in->mutex, &attr2);

	m_handle = in;
}

Event::~Event()
{
	Internal* in = static_cast< Internal* >(m_handle);
	assert (in);

	sys_cond_destroy(in->cond);
	sys_mutex_destroy(in->mutex);

	delete in;
}

void Event::pulse(int count)
{
	Internal* in = static_cast< Internal* >(m_handle);
	assert (in);

	sys_mutex_lock(in->mutex, SYS_NO_TIMEOUT);
	sys_cond_signal(in->cond);
	sys_mutex_unlock(in->mutex);
}

void Event::broadcast()
{
	Internal* in = static_cast< Internal* >(m_handle);
	assert (in);

	sys_mutex_lock(in->mutex, SYS_NO_TIMEOUT);
	sys_cond_signal(in->cond);
	sys_mutex_unlock(in->mutex);
}

void Event::reset()
{
	assert (0);
}

bool Event::wait(int32_t timeout)
{
	Internal* in = static_cast< Internal* >(m_handle);
	assert (in);

	int rc = sys_mutex_lock(in->mutex, SYS_NO_TIMEOUT);
	if (rc != CELL_OK)
		return false;

	rc = sys_cond_wait(in->cond, timeout);
	sys_mutex_unlock(in->mutex);
	
	return bool(rc == CELL_OK);
}

}
