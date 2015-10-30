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
#if !defined(_XBOX)
	MMRESULT result = timeBeginPeriod(1);
#endif

	bool ret = bool(WaitForSingleObject(m_handle, (timeout < 0) ? INFINITE : timeout) == WAIT_OBJECT_0);

#if !defined(_XBOX)
	if (result == TIMERR_NOERROR)
		timeEndPeriod(1);
#endif

	return ret;
}

}
