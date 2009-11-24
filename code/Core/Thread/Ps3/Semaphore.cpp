#include <sys/synchronization.h>
#include "Core/Thread/Semaphore.h"
#include "Core/Misc/TString.h"

namespace traktor
{

Semaphore::Semaphore()
:	m_handle(0)
{
	//sys_semaphore_t* sem = new sys_semaphore_t();

	//sys_semaphore_attribute_t attr;
	//sys_semaphore_attribute_initialize(attr);
	//sys_semaphore_create(sem, &attr, 1, 1024);

	//m_handle = sem;

	sys_mutex_t* mutex = new sys_mutex_t();

	sys_mutex_attribute_t attr;
	sys_mutex_attribute_initialize(attr);
	sys_mutex_create(mutex, &attr);

	m_handle = mutex;
}

Semaphore::~Semaphore()
{
	//sys_semaphore_t* sem = reinterpret_cast< sys_semaphore_t* >(m_handle);
	//sys_semaphore_destroy(*sem);
	//delete sem;

	sys_mutex_t* mutex = reinterpret_cast< sys_mutex_t* >(m_handle);
	sys_mutex_destroy(*mutex);
	delete mutex;
}

bool Semaphore::wait(int32_t timeout)
{
	//sys_semaphore_t* sem = reinterpret_cast< sys_semaphore_t* >(m_handle);
	//return sys_semaphore_wait(*sem, timeout * 1000) == CELL_OK;

	sys_mutex_t* mutex = reinterpret_cast< sys_mutex_t* >(m_handle);
	int rc = sys_mutex_lock(*mutex, timeout);
	return bool(rc == CELL_OK);
}

void Semaphore::release()
{
	//sys_semaphore_t* sem = reinterpret_cast< sys_semaphore_t* >(m_handle);
	//sys_semaphore_post(*sem, 1);

	sys_mutex_t* mutex = reinterpret_cast< sys_mutex_t* >(m_handle);
	sys_mutex_unlock(*mutex);
}

}
