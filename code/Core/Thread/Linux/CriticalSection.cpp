#include <mutex>
#include "Core/Thread/CriticalSection.h"

namespace traktor
{
	namespace
	{

struct InternalData
{
	std::timed_mutex mtx;
};

	}

CriticalSection::CriticalSection()
{
	InternalData* data = new InternalData();
	m_handle = data;
}

CriticalSection::~CriticalSection()
{
	delete reinterpret_cast< InternalData* >(m_handle);
}

bool CriticalSection::wait(int32_t timeout)
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

void CriticalSection::release()
{
	InternalData* data = reinterpret_cast< InternalData* >(m_handle);
	data->mtx.unlock();
}

}
