#include <pthread.h>
#include <sys/time.h>
#include "Core/Misc/TString.h"
#include "Core/Thread/Atomic.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{
	namespace
	{

struct InternalData
{
	int32_t count;
	int32_t value;
	pthread_t owner;
	Event event;
};

	}

Semaphore::Semaphore()
:	m_handle(0)
{
	InternalData* in = new InternalData();

	in->count = 0;
	in->value = 0;
	in->owner = 0;

	m_handle = in;
}

Semaphore::~Semaphore()
{
	InternalData* in = reinterpret_cast< InternalData* >(m_handle);
	delete in;
}

bool Semaphore::wait(int32_t timeout)
{
	InternalData* in = reinterpret_cast< InternalData* >(m_handle);

	pthread_t current = pthread_self();
	T_ASSERT (current != 0);

	if (Atomic::increment(in->value) > 1)
	{
		if (pthread_equal(in->owner, current) != 0)
		{
			T_ASSERT (in->count > 0);
			in->count++;
			return true;
		}

		if (!in->event.wait(timeout))
		{
			Atomic::decrement(in->value);
			return false;
		}
	}

	T_ASSERT (in->owner == 0);
	in->owner = current;
	in->count = 1;

	return true;
}

void Semaphore::release()
{
	InternalData* in = reinterpret_cast< InternalData* >(m_handle);
	T_ASSERT (pthread_equal(in->owner, pthread_self()) != 0);

	if (--in->count > 0)
	{
		Atomic::decrement(in->value);
	}
	else
	{
		in->owner = 0;
		if (Atomic::decrement(in->value) > 0)
			in->event.pulse();
	}
}

}
