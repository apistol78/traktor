/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Io/StringOutputStream.h>
#include <Core/Misc/TString.h>
#include "Drone/App/Process.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drone.Process", Process, Object)

Process::Process()
{
	std::memset(&m_pi, 0, sizeof(m_pi));
}

Process::~Process()
{
	if (m_pi.hProcess)
		CloseHandle(m_pi.hProcess);
}

bool Process::create(
	const Path& command,
	const std::wstring& commandArguments,
	const Path& workingDirectory
)
{
	STARTUPINFO si;

	std::memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);

	StringOutputStream commandLine;
	commandLine << command.getPathName() << L" " << commandArguments;

	TCHAR szCommand[1024];
	_tcscpy_s(szCommand, wstots(commandLine.str()).c_str());

	TCHAR szWorkingDirectory[MAX_PATH];
	_tcscpy_s(szWorkingDirectory, wstots(workingDirectory.getPathName()).c_str());

	BOOL result = CreateProcess(
		NULL,
		szCommand,
		NULL,
		NULL,
		TRUE,
		CREATE_NEW_CONSOLE,
		NULL,
		szWorkingDirectory,
		&si,
		&m_pi
	);
	if (!result)
		return false;

	return true;
}

bool Process::create(
	const Path& command,
	const std::wstring& commandArguments
)
{
	STARTUPINFO si;

	std::memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);

	StringOutputStream commandLine;
	commandLine << command.getPathName() << L" " << commandArguments;

	TCHAR szCommand[1024];
	_tcscpy_s(szCommand, wstots(commandLine.str()).c_str());

	BOOL result = CreateProcess(
		NULL,
		szCommand,
		NULL,
		NULL,
		TRUE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&si,
		&m_pi
	);
	if (!result)
		return false;

	return true;
}

bool Process::wait(int timeout)
{
	return bool(WaitForSingleObject(m_pi.hProcess, timeout >= 0 ? timeout : INFINITE) == WAIT_OBJECT_0);
}

bool Process::finished()
{
	return wait(0);
}

uint32_t Process::exitCode()
{
	DWORD dwExitCode;
	if (!GetExitCodeProcess(m_pi.hProcess, &dwExitCode))
		return 0;

	return uint32_t(dwExitCode);
}

	}
}
