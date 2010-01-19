#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <lmcons.h>
#include <shlobj.h>
#include <tchar.h>
#include "Core/Io/IVolume.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/System/OS.h"
#include "Core/System/Win32/ProcessWin32.h"
#include "Core/System/Win32/SharedMemoryWin32.h"

namespace traktor
{
	namespace
	{

typedef HRESULT STDAPICALLTYPE IEISPROTECTEDMODEPROCESSPROC (BOOL* pbResult);
typedef HRESULT STDAPICALLTYPE IEGETWRITEABLEFOLDERPATHPROC (REFGUID clsidFolderID, LPWSTR *lppwstrPath);

HINSTANCE s_hIeFrameLib = 0;
IEISPROTECTEDMODEPROCESSPROC* s_IEIsProtectedModeProcess = 0;
IEGETWRITEABLEFOLDERPATHPROC* s_IEGetWriteableFolderPath = 0;

	}

OS& OS::getInstance()
{
	static OS* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new OS();
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

uint32_t OS::getCPUCoreCount() const
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
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

std::wstring OS::getUserHomePath() const
{
#if !defined(WINCE)
	TCHAR szPath[MAX_PATH];
	HRESULT hr;

	hr = SHGetFolderPath(
		NULL,
		CSIDL_PERSONAL,
		NULL,
		SHGFP_TYPE_CURRENT,
		szPath
	);
	if (FAILED(hr))
		return L"";

	return replaceAll(tstows(szPath), L'\\', L'/');
#else
	return L"";
#endif
}

std::wstring OS::getUserApplicationDataPath() const
{
#if !defined(WINCE)
	TCHAR szPath[MAX_PATH];
	HRESULT hr;

	hr = SHGetFolderPath(
		NULL,
		CSIDL_LOCAL_APPDATA,
		NULL,
		SHGFP_TYPE_CURRENT,
		szPath
	);
	if (FAILED(hr))
		return L"";

	return replaceAll(tstows(szPath), L'\\', L'/');
#else
	return L"";
#endif
}

std::wstring OS::getWritableFolderPath() const
{
#if !defined(WINCE)
	if (s_IEIsProtectedModeProcess && s_IEGetWriteableFolderPath)
	{
		HRESULT hr;
		BOOL pm;

		hr = (*s_IEIsProtectedModeProcess)(&pm);
		if (FAILED(hr))
			pm = FALSE;

		if (pm)
		{
			LPWSTR pwstrPath = 0;
			hr = (*s_IEGetWriteableFolderPath)(
				FOLDERID_LocalAppDataLow,
				&pwstrPath
			);
			if (FAILED(hr))
				return L"";

			std::wstring path = pwstrPath;
			path = replaceAll(path, L'\\', L'/');

			CoTaskMemFree(pwstrPath);
			return path;
		}
	}
#endif
	return getUserApplicationDataPath();
}

bool OS::editFile(const Path& file) const
{
#if !defined(WINCE)
	Path absoluteFile = FileSystem::getInstance().getAbsolutePath(file);
	HINSTANCE hInstance = ShellExecute(
		NULL,
		_T("edit"),
		wstots(absoluteFile.getPathName()).c_str(),
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
		wstots(absoluteFile.getPathName()).c_str(),
		NULL,
		NULL,
		SW_SHOWDEFAULT
	);
	return int(hInstance) > 32;
#else
	return false;
#endif
}

Ref< IProcess > OS::execute(const Path& file, const std::wstring& commandLine, const Path& workingDirectory, bool redirect, bool mute) const
{
	TCHAR cmd[32768], cwd[MAX_PATH];
	HANDLE hStdInRead = 0, hStdInWrite = 0;
	HANDLE hStdOutRead = 0, hStdOutWrite = 0;
	HANDLE hStdErrRead = 0, hStdErrWrite = 0;

	Path fileAbsolute = FileSystem::getInstance().getAbsolutePath(file);

	StringOutputStream ss;
	ss << L"\"" << fileAbsolute.getPathName() << L"\"";
	if (!commandLine.empty())
		ss << L" " << commandLine;

#if !defined(WINCE)
	_tcscpy_s(cmd, wstots(ss.str()).c_str());
	_tcscpy_s(cwd, wstots(workingDirectory.getPathName()).c_str());
#else
	_tcscpy_s(cmd, sizeof_array(cmd), wstots(ss.str()).c_str());
	_tcscpy_s(cwd, sizeof_array(cwd), wstots(workingDirectory.getPathName()).c_str());
#endif

#if !defined(WINCE)
	if (redirect)
	{
		// Create IO pipes.
		SECURITY_DESCRIPTOR sd;
		InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
		SetSecurityDescriptorDacl(&sd, true, NULL, false);
	
		SECURITY_ATTRIBUTES sa;
		std::memset(&sa, 0, sizeof(sa));
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = &sd;
		sa.bInheritHandle = true;
	
		CreatePipe(
			&hStdInRead,
			&hStdInWrite,
			&sa,
			0
		);
	
		CreatePipe(
			&hStdOutRead,
			&hStdOutWrite,
			&sa,
			0
		);
	
		CreatePipe(
			&hStdErrRead,
			&hStdErrWrite,
			&sa,
			0
		);
	}
#endif

	STARTUPINFO si;
	std::memset(&si, 0, sizeof(si));
	si.cb = sizeof(STARTUPINFO);
#if !defined(WINCE)
	si.dwFlags = redirect ? STARTF_USESTDHANDLES : 0;
#else
	si.dwFlags = 0;
#endif
	si.hStdInput = hStdInRead;
	si.hStdOutput = hStdOutWrite;
	si.hStdError = hStdErrWrite;

	PROCESS_INFORMATION pi;
	std::memset(&pi, 0, sizeof(pi));

	DWORD dwCreationFlags;
#if !defined(WINCE)
	dwCreationFlags = mute ? CREATE_NO_WINDOW : CREATE_NEW_CONSOLE;
#else
	dwCreationFlags = mute ? 0 : CREATE_NEW_CONSOLE;
#endif

	BOOL result = CreateProcess(
		NULL,
		cmd,
		NULL,
		NULL,
		TRUE,
		dwCreationFlags,
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

	return new ProcessWin32(
		pi,
		hStdInRead,
		hStdInWrite,
		hStdOutRead,
		hStdOutWrite,
		hStdErrRead,
		hStdErrWrite
	);
}

Ref< ISharedMemory > OS::createSharedMemory(const std::wstring& name, uint32_t size) const
{
	Ref< SharedMemoryWin32 > sharedMemory = new SharedMemoryWin32();
	if (!sharedMemory->create(name, size))
		return 0;
	return sharedMemory;
}

OS::OS()
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

#if !defined(WINCE)
	// Load IEFrame library; only available on Vista.
	s_hIeFrameLib = LoadLibrary(L"ieframe.dll");
	if (s_hIeFrameLib)
	{
		s_IEIsProtectedModeProcess = (IEISPROTECTEDMODEPROCESSPROC*)GetProcAddress(s_hIeFrameLib, "IEIsProtectedModeProcess");
		T_ASSERT (s_IEIsProtectedModeProcess);
		s_IEGetWriteableFolderPath = (IEGETWRITEABLEFOLDERPATHPROC*)GetProcAddress(s_hIeFrameLib, "IEGetWriteableFolderPath");
		T_ASSERT (s_IEGetWriteableFolderPath);
	}
#endif
}

OS::~OS()
{
	if (s_hIeFrameLib)
		FreeLibrary(s_hIeFrameLib);

	CoUninitialize();
}

void OS::destroy()
{
	delete this;
}

}
