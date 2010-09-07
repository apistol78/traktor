#include <sys/synchronization.h>
#include "Core/Thread/CriticalSection.h"

namespace traktor
{

CriticalSection::CriticalSection()
:	m_handle(0)
{
	sys_lwmutex_attribute_t attr;
	sys_lwmutex_attribute_initialize(attr);

	sys_lwmutex_t* lwm = new sys_lwmutex_t();
	sys_lwmutex_create(lwm, &attr);

	m_handle = lwm;
}

CriticalSection::~CriticalSection()
{
	sys_lwmutex_t* mutex = reinterpret_cast< sys_lwmutex_t* >(m_handle);
	sys_lwmutex_destroy(mutex);

	delete mutex;
}

bool CriticalSection::wait(int32_t timeout)
{
	sys_lwmutex_t* mutex = reinterpret_cast< sys_lwmutex_t* >(m_handle);
	if (timeout >= 0)
	{
		int rc = sys_lwmutex_lock(mutex, usecond_t(timeout) * 1000);
		return bool(rc == CELL_OK);
	}
	else
	{
		while (sys_lwmutex_lock(mutex, SYS_NO_TIMEOUT) != CELL_OK)
			;
		return true;
	}
}

void CriticalSection::release()
{
	sys_lwmutex_t* mutex = reinterpret_cast< sys_lwmutex_t* >(m_handle);
	sys_lwmutex_unlock(mutex);
}

}

