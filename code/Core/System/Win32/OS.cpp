#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <lmcons.h>
#include <tchar.h>
#include "Core/System/OS.h"
#include "Core/System/Win32/ProcessWin32.h"
#include "Core/System/Win32/SharedMemoryWin32.h"
#include "Core/Heap/Heap.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Io/FileSystem.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.OS", OS, Singleton)

OS& OS::getInstance()
{
	static OS* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new OS();
		SingletonManager::getInstance().addBefore(s_instance, &Heap::getInstance());
	}
	return *s_instance;
}

std::wstring OS::getComputerName() const
{
#if !defined(WINCE)
	wchar_t buf[MAX_COMPUTERNAME_LENGTH + 1];

	DWORD size = sizeof_array(buf);
	if (!GetComputerName(buf, &size))
		return L"Unavailable";

	return buf;
#else
	return L"Unavailable";
#endif
}

std::wstring OS::getCurrentUser() const
{
#if !defined(WINCE)
	wchar_t buf[UNLEN + 1];
	
	DWORD size = sizeof_array(buf);
	if (!GetUserName(buf, &size))
		return L"Unavailable";

	return buf;
#else
	return L"Unavailable";
#endif
}

bool OS::editFile(const Path& file) const
{
#if !defined(WINCE)
	Path absoluteFile = FileSystem::getInstance().getAbsolutePath(file);
	HINSTANCE hInstance = ShellExecute(
		NULL,
		_T("edit"),
		wstots(absoluteFile).c_str(),
		NULL,
		NULL,
		SW_SHOWDEFAULT
	);
	return int(hInstance) > 32;
#else
	return false;
#endif
}

bool OS::exploreFile(const Path& file) const
{
#if !defined(WINCE)
	Path absoluteFile = FileSystem::getInstance().getAbsolutePath(file);
	HINSTANCE hInstance = ShellExecute(
		NULL,
		_T("explore"),
		wstots(absoluteFile).c_str(),
		NULL,
		NULL,
		SW_SHOWDEFAULT
	);
	return int(hInstance) > 32;
#else
	return false;
#endif
}

Process* OS::execute(const Path& file, const std::wstring& commandLine, const Path& workingDirectory) const
{
	STARTUPINFO si;
	std::memset(&si, 0, sizeof(si));
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi;
	std::memset(&pi, 0, sizeof(pi));

	TCHAR app[MAX_PATH], cmd[32768], cwd[MAX_PATH];
#if !defined(WINCE)
	_tcscpy_s(app, wstots(file).c_str());
	_tcscpy_s(cmd, wstots(commandLine).c_str());
	_tcscpy_s(cwd, wstots(workingDirectory).c_str());
#else
	_tcscpy_s(app, sizeof_array(app), wstots(file).c_str());
	_tcscpy_s(cmd, sizeof_array(cmd), wstots(commandLine).c_str());
	_tcscpy_s(cwd, sizeof_array(cwd), wstots(workingDirectory).c_str());
#endif

	BOOL result = CreateProcess(
		app,
		(cmd[0] != L'\0' ? cmd : NULL),
		NULL,
		NULL,
		TRUE,
		CREATE_NEW_CONSOLE,
		NULL,
		(cwd[0] != L'\0' ? cwd : NULL),
		&si,
		&pi
	);
	if (result == FALSE)
	{
		log::debug << L"Unable to create process, error = " << (int32_t)GetLastError() << Endl;
		return 0;
	}

	return gc_new< ProcessWin32 >(cref(pi));
}

SharedMemory* OS::createSharedMemory(const std::wstring& name, uint32_t size) const
{
	Ref< SharedMemoryWin32 > sharedMemory = gc_new< SharedMemoryWin32 >();
	if (!sharedMemory->create(name, size))
		return 0;
	return sharedMemory;
}

OS::OS()
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
}

OS::~OS()
{
	CoUninitialize();
}

void OS::destroy()
{
	delete this;
}

}
