#include "Core/Thread/Semaphore.h"

namespace traktor
{

Semaphore::Semaphore()
:	m_handle(0)
{
}

Semaphore::~Semaphore()
{
}

bool Semaphore::wait(int32_t timeout)
{
	return true;
}

void Semaphore::release()
{
}

}
