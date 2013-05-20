#include <sys/types.h>
#include <sys/wait.h>
#include "Core/System/Emscripten/ProcessEmscripten.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ProcessEmscripten", ProcessEmscripten, IProcess)

Ref< IStream > ProcessEmscripten::getPipeStream(StdPipe pipe)
{
	return 0;
}

bool ProcessEmscripten::signal(SignalType signalType)
{
	return false;
}

int32_t ProcessEmscripten::exitCode() const
{
	return m_exitCode;
}

bool ProcessEmscripten::wait(int32_t timeout)
{
	if (timeout >= 0)
	{
		for (;;)
		{
			if (waitpid(m_pid, &m_exitCode, WNOHANG) > 0)
				return true;

			if ((timeout -= 10) < 0)
				return false;

			// usleep(10 * 1000);
		}
	}
	else
		return waitpid(m_pid, &m_exitCode, 0) >= 0;
}

ProcessEmscripten::ProcessEmscripten(pid_t pid)
:	m_pid(pid)
,	m_exitCode(0)
{
}

}
