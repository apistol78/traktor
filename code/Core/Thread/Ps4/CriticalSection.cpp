#include "Core/Thread/CriticalSection.h"

namespace traktor
{

CriticalSection::CriticalSection()
{
	ScePthreadMutexattr attr;
	scePthreadMutexattrInit(&attr);
	scePthreadMutexInit(&m_mutex, &attr, 0);
}

CriticalSection::~CriticalSection()
{
	scePthreadMutexDestroy(&m_mutex);
}

bool CriticalSection::wait(int32_t timeout)
{
	if (timeout >= 0)
		return scePthreadMutexTimedlock(&m_mutex, timeout * 1000) == 0;
	else
		return scePthreadMutexLock(&m_mutex) == 0;
}

void CriticalSection::release()
{
	scePthreadMutexUnlock(&m_mutex);
}

}

