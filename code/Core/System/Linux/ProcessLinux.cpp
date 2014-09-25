#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "Core/System/Linux/ProcessLinux.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ProcessLinux", ProcessLinux, IProcess)

Ref< IStream > ProcessLinux::getPipeStream(StdPipe pipe)
{
	return 0;
}

bool ProcessLinux::signal(SignalType signalType)
{
	return false;
}

bool ProcessLinux::terminate(int32_t exitCode)
{
	return false;
}

int32_t ProcessLinux::exitCode() const
{
	return m_exitCode;
}

bool ProcessLinux::wait(int32_t timeout)
{
	if (timeout >= 0)
	{
		for (;;)
		{
			if (waitpid(m_pid, &m_exitCode, WNOHANG) > 0)
				return true;

			if ((timeout -= 10) < 0)
				return false;

			usleep(10 * 1000);
		}
	}
	else
		return waitpid(m_pid, &m_exitCode, 0) >= 0;
}

ProcessLinux::ProcessLinux(pid_t pid)
:	m_pid(pid)
,	m_exitCode(0)
{
}

}
