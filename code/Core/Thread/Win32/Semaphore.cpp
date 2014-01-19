#include "Core/Platform.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{

Semaphore::Semaphore()
{
	m_handle = CreateMutex(0, FALSE, NULL);
}

Semaphore::~Semaphore()
{
	CloseHandle(m_handle);
}

bool Semaphore::wait(int32_t timeout)
{
	return bool(WaitForSingleObject(m_handle, (timeout < 0) ? INFINITE : timeout) == WAIT_OBJECT_0);
}

void Semaphore::release()
{
	ReleaseMutex(m_handle);
}

}
