#include "Core/Platform.h"
#include "Core/Thread/Signal.h"

namespace traktor
{

Signal::Signal()
{
	m_handle = CreateEvent(NULL, TRUE, FALSE, NULL);
}

Signal::~Signal()
{
	CloseHandle(m_handle);
}

void Signal::set()
{
	SetEvent(m_handle);
}

void Signal::reset()
{
	ResetEvent(m_handle);
}

bool Signal::wait(int timeout)
{
	return bool(WaitForSingleObject(m_handle, timeout >= 0 ? timeout : INFINITE) == WAIT_OBJECT_0);
}

}
