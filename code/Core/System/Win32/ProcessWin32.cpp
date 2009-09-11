#include "Core/System/Win32/ProcessWin32.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ProcessWin32", ProcessWin32, Process)

ProcessWin32::ProcessWin32(const PROCESS_INFORMATION& pi)
:	m_pi(pi)
{
}

ProcessWin32::~ProcessWin32()
{
	CloseHandle(m_pi.hThread);
	CloseHandle(m_pi.hProcess);
}

bool ProcessWin32::wait(int32_t timeout)
{
	return WaitForSingleObject(m_pi.hProcess, timeout >= 0 ? timeout : INFINITE) == WAIT_OBJECT_0;
}

int32_t ProcessWin32::exitCode() const
{
	DWORD code;

	if (!GetExitCodeProcess(m_pi.hProcess, &code))
		return 0;

	return (int32_t)code;
}

}
