#include <sys/synchronization.h>
#include "Core/Thread/Semaphore.h"
#include "Core/Misc/TString.h"

namespace traktor
{

Semaphore::Semaphore()
:	m_handle(0)
{
	sys_lwmutex_attribute_t attr;
	sys_lwmutex_attribute_initialize(attr);
	attr.attr_recursive = SYS_SYNC_RECURSIVE;

	sys_lwmutex_t* lwm = new sys_lwmutex_t();
	sys_lwmutex_create(lwm, &attr);

	m_handle = lwm;
}

Semaphore::~Semaphore()
{
	sys_lwmutex_t* mutex = reinterpret_cast< sys_lwmutex_t* >(m_handle);
	sys_lwmutex_destroy(mutex);

	delete mutex;
}

bool Semaphore::wait(int32_t timeout)
{
	sys_lwmutex_t* mutex = reinterpret_cast< sys_lwmutex_t* >(m_handle);
	int rc = sys_lwmutex_lock(mutex, timeout);
	return bool(rc == CELL_OK);
}

void Semaphore::release()
{
	sys_lwmutex_t* mutex = reinterpret_cast< sys_lwmutex_t* >(m_handle);
	sys_lwmutex_unlock(mutex);
}

}
