#include <cassert>
#include <sys/synchronization.h>
#include "Core/Thread/Mutex.h"

namespace traktor
{

Mutex::Mutex()
:	m_existing(false)
,	m_handle(0)
{
	sys_mutex_t* mutex = new sys_mutex_t();

	sys_mutex_attribute_t attr;
	sys_mutex_attribute_initialize(attr);
	sys_mutex_create(mutex, &attr);

	m_handle = mutex;
}

Mutex::Mutex(const Guid& id)
:	m_existing(false)
,	m_handle(0)
{
	T_FATAL_ERROR;
}

Mutex::~Mutex()
{
	sys_mutex_t* mutex = reinterpret_cast< sys_mutex_t* >(m_handle);
	sys_mutex_destroy(*mutex);
	delete mutex;
}

bool Mutex::wait(int32_t timeout)
{
	sys_mutex_t* mutex = reinterpret_cast< sys_mutex_t* >(m_handle);
	int rc = sys_mutex_lock(*mutex, usecond_t(timeout) * 1000);
	return bool(rc == CELL_OK);
}

void Mutex::release()
{
	sys_mutex_t* mutex = reinterpret_cast< sys_mutex_t* >(m_handle);
	sys_mutex_unlock(*mutex);
}

bool Mutex::existing() const
{
	return m_existing;
}

}
