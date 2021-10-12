#include <mutex>
#include "Core/Thread/Semaphore.h"

namespace traktor
{
	namespace
	{

struct InternalData
{
	std::recursive_timed_mutex mtx;
};

	}

Semaphore::Semaphore()
:	m_handle(nullptr)
{
	InternalData* data = new InternalData();
	m_handle = data;
}

Semaphore::~Semaphore()
{
	delete reinterpret_cast< InternalData* >(m_handle);
}

bool Semaphore::wait(int32_t timeout)
{
	InternalData* data = reinterpret_cast< InternalData* >(m_handle);
	if (timeout < 0)
	{
		data->mtx.lock();
	}
	else
	{
		if (!data->mtx.try_lock_for(std::chrono::milliseconds(timeout)))
			return false;
	}
	return true;
}

void Semaphore::release()
{
	InternalData* data = reinterpret_cast< InternalData* >(m_handle);
	data->mtx.unlock();
}

}
