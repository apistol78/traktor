#include <unistd.h>
#include "Core/System/Osx/ProcessOsX.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ProcessOsX", ProcessOsX, IProcess)

Ref< IStream > ProcessOsX::getPipeStream(StdPipe pipe)
{
	return 0;
}

bool ProcessOsX::signal(SignalType signalType)
{
	return false;
}
	
int32_t ProcessOsX::exitCode() const
{
	return m_exitCode;
}
	
bool ProcessOsX::wait(int32_t timeout)
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

ProcessOsX::ProcessOsX(pid_t pid)
:	m_pid(pid)
,	m_exitCode(0)
{
}

}

