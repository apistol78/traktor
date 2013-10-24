#include <pthread.h>
#include "Core/Thread/CriticalSection.h"

namespace traktor
{

CriticalSection::CriticalSection()
{
	pthread_mutex_t* mutex = new pthread_mutex_t();

	int rc = pthread_mutex_init(mutex, 0);
	T_ASSERT (rc == 0);

	m_handle = mutex;
}

CriticalSection::~CriticalSection()
{
	delete reinterpret_cast< pthread_mutex_t* >(m_handle);
}

bool CriticalSection::wait(int32_t timeout)
{
	pthread_mutex_t* mutex = reinterpret_cast< pthread_mutex_t* >(m_handle);
	while (pthread_mutex_lock(mutex) != 0)
		;
	return true;
}

void CriticalSection::release()
{
	pthread_mutex_t* mutex = reinterpret_cast< pthread_mutex_t* >(m_handle);
	int rc = pthread_mutex_unlock(mutex);
	T_ASSERT (rc == 0);
}

}
