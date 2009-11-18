#include "Core/Platform.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{
	namespace
	{

struct Internal
{
	LONG owner;
	LONG count;
	LONG value;
	HANDLE handle;
};

	}

Semaphore::Semaphore()
{
	HANDLE handle = CreateEvent(NULL, FALSE, FALSE, NULL);
	T_FATAL_ASSERT_M (handle != NULL, L"Unable to create event");

	Internal* in = new Internal();
	T_FATAL_ASSERT_M (in, L"Allocation failed");

	in->owner = ~0;
	in->count = 0;
	in->value = 0;
	in->handle = handle;

	m_handle = in;
}

Semaphore::~Semaphore()
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	CloseHandle(in->handle);
	delete in;
}

bool Semaphore::wait(int32_t timeout)
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);

	long threadId = GetCurrentThreadId();

	if (InterlockedIncrement(&in->value) > 1)
	{
		if (in->owner == threadId)
		{
			T_ASSERT (in->count > 0);
			in->count++;
			return true;
		}

		if (WaitForSingleObject(in->handle, timeout) != WAIT_OBJECT_0)
		{
			InterlockedDecrement(&in->value);
			return false;
		}
	}

	T_ASSERT (in->owner == ~0);
	in->owner = threadId;
	in->count = 1;

	return true;
}

void Semaphore::release()
{
	Internal* in = reinterpret_cast< Internal* >(m_handle);
	T_ASSERT (in->owner == GetCurrentThreadId());

	if (--in->count > 0)
	{
		InterlockedDecrement(&in->value);
	}
	else
	{
		in->owner = ~0;
		if (InterlockedDecrement(&in->value) > 0)
			SetEvent(in->handle);
	}
}

}
