/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/System/Win32/ProcessShellWin32.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ProcessShellWin32", ProcessShellWin32, IProcess)

ProcessShellWin32::ProcessShellWin32(HANDLE hProcess)
:	m_hProcess(hProcess)
{
}

ProcessShellWin32::~ProcessShellWin32()
{
	CloseHandle(m_hProcess);
}

bool ProcessShellWin32::setPriority(Priority priority)
{
	return false;
}

bool ProcessShellWin32::wait(int32_t timeout)
{
	return WaitForSingleObject(m_hProcess, timeout >= 0 ? timeout : INFINITE) == WAIT_OBJECT_0;
}

IStream* ProcessShellWin32::getPipeStream(StdPipe pipe)
{
	return nullptr;
}

IProcess::WaitPipeResult ProcessShellWin32::waitPipeStream(int32_t timeout, Ref< IStream >& outPipe)
{
	return wait(timeout) ? Terminated : Timeout;
}

bool ProcessShellWin32::signal(SignalType signalType)
{
	return false;
}

bool ProcessShellWin32::terminate(int32_t exitCode)
{
	return false;
}

int32_t ProcessShellWin32::exitCode() const
{
	DWORD code = 0;

	if (!GetExitCodeProcess(m_hProcess, &code))
		return 0;

	return (int32_t)code;
}

}
