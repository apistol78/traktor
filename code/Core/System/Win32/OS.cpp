/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <lmcons.h>
#include <shlobj.h>
#include <tchar.h>
#include "Core/Io/IVolume.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/TString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/System/Environment.h"
#include "Core/System/OS.h"
#include "Core/System/ResolveEnv.h"
#include "Core/System/Win32/ProcessWin32.h"
#include "Core/System/Win32/ProcessShellWin32.h"
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.OS", OS, Object)

OS& OS::getInstance()
{
	static OS* s_instance = nullptr;
	if (!s_instance)
	{
		s_instance = new OS();
		s_instance->addRef(0);
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

std::wstring OS::getName() const
{
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO *) &osvi);

	if (osvi.wProductType == VER_NT_WORKSTATION)
	{
		if (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0)
			return L"Windows 10";
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3)
			return L"Windows 8.1";
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2)
			return L"Windows 8";
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
			return L"Windows 7";
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
			return L"Windows Vista";
		else
			return str(L"Windows NT (%d.%d)", osvi.dwMajorVersion, osvi.dwMinorVersion);
	}
	else
	{
		if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3)
			return L"Windows Server 2012 R2";
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2)
			return L"Windows Server 2012";
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
			return L"Windows Server 2008 R2";
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
			return L"Windows Server 2008";
		else
			return str(L"Windows Server (%d.%d)", osvi.dwMajorVersion, osvi.dwMinorVersion);
	}
}

std::wstring OS::getIdentifier() const
{
	return L"windows";
}

uint32_t OS::getCPUCoreCount() const
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}

Path OS::getExecutable() const
{
	wchar_t fileName[MAX_PATH];
	GetModuleFileName(NULL, fileName, MAX_PATH);
	return Path(fileName);
}

std::wstring OS::getCommandLine() const
{
	return tstows(GetCommandLine());
}

std::wstring OS::getComputerName() const
{
	wchar_t buf[MAX_COMPUTERNAME_LENGTH + 1];

	DWORD size = sizeof_array(buf);
	if (!GetComputerName(buf, &size))
		return L"";

	return buf;
}

std::wstring OS::getCurrentUser() const
{
	wchar_t buf[UNLEN + 1];

	DWORD size = sizeof_array(buf);
	if (!GetUserName(buf, &size))
		return L"";

	return buf;
}

std::wstring OS::getUserHomePath() const
{
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
}

std::wstring OS::getUserApplicationDataPath() const
{
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
}

std::wstring OS::getWritableFolderPath() const
{
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
	return getUserApplicationDataPath();
}

bool OS::openFile(const std::wstring& file) const
{
	HINSTANCE hInstance = ShellExecute(
		NULL,
		_T("open"),
		wstots(replaceAll(file, L"/", L"\\")).c_str(),
		NULL,
		NULL,
		SW_SHOWDEFAULT
	);
	return intptr_t(hInstance) > 32;
}

bool OS::editFile(const std::wstring& file) const
{
	HINSTANCE hInstance = ShellExecute(
		NULL,
		_T("open"),
		wstots(replaceAll(file, L"/", L"\\")).c_str(),
		NULL,
		NULL,
		SW_SHOWDEFAULT
	);
	return intptr_t(hInstance) > 32;
}

bool OS::exploreFile(const std::wstring& file) const
{
	HINSTANCE hInstance = ShellExecute(
		NULL,
		_T("explore"),
		wstots(replaceAll(file, L"/", L"\\")).c_str(),
		NULL,
		NULL,
		SW_SHOWDEFAULT
	);
	return intptr_t(hInstance) > 32;
}

bool OS::setEnvironment(const std::wstring& name, const std::wstring& value) const
{
	return SetEnvironmentVariable(wstots(name).c_str(), wstots(value).c_str()) != FALSE;
}

Ref< Environment > OS::getEnvironment() const
{
	Ref< Environment > env = new Environment();
	LPTCH lpEnv = GetEnvironmentStrings();
	if (lpEnv)
	{
		TCHAR* p = lpEnv;
		while (*p != 0)
		{
			TCHAR* key = p;
			TCHAR* sep = _tcschr(p, '=');
			if (!sep)
				break;
			TCHAR* val = sep + 1;

			if (key < sep)
			{
				env->set(
					tstows(tstring(key, sep)),
					tstows(val)
				);
			}

			p = val + _tcslen(val) + 1;
		}
	}
	return env;
}

bool OS::getEnvironment(const std::wstring& name, std::wstring& outValue) const
{
	TCHAR buf[32767] = { 0 };
	if (GetEnvironmentVariable(wstots(name).c_str(), buf, sizeof_array(buf)) == 0)
	{
		if (GetLastError() == ERROR_ENVVAR_NOT_FOUND)
			return false;
	}
	outValue = tstows(buf);
	return true;
}

Ref< IProcess > OS::execute(
	const std::wstring& commandLine,
	const Path& workingDirectory,
	const Environment* env,
	uint32_t flags
) const
{
	AutoArrayPtr< char > environment;
	std::wstring executable;
	std::wstring arguments;

	// Resolve entire command line.
	const std::wstring resolvedCommandLine = resolveEnv(commandLine, env);

	// Extract executable file from command line.
	if (resolvedCommandLine.empty())
		return nullptr;

	if (resolvedCommandLine[0] == L'\"')
	{
		const size_t i = resolvedCommandLine.find(L'\"', 1);
		if (i == resolvedCommandLine.npos)
			return nullptr;

		executable = resolvedCommandLine.substr(1, i - 1);
		arguments = resolvedCommandLine.substr(i + 1);
	}
	else
	{
		const size_t i = resolvedCommandLine.find(L' ');
		if (i != resolvedCommandLine.npos)
		{
			executable = resolvedCommandLine.substr(0, i);
			arguments = resolvedCommandLine.substr(i + 1);
		}
		else
			executable = resolvedCommandLine;
	}

	const Path executablePath = executable;
	const Path workingDirectoryAbs = FileSystem::getInstance().getAbsolutePath(workingDirectory);

	// Create environment variables.
	if (env)
	{
		const std::map< std::wstring, std::wstring >& e = env->get();

		// Calculate how much space we need to allocate.
		uint32_t size = 0;
		for (auto i = e.begin(); i != e.end(); ++i)
			size += (uint32_t)(i->first.length() + 1 + i->second.length() + 1);
		size += 1;

		environment.reset(new char [size]);

		char* p = environment.ptr(); *p = 0;
		for (auto i = e.begin(); i != e.end(); ++i)
		{
			strcpy(p, wstombs(i->first).c_str());
			strcat(p, "=");
			strcat(p, wstombs(i->second).c_str());
			p += i->first.length() + 1 + i->second.length() + 1;
		}
		*p++ = 0;

		T_ASSERT(size_t(p - environment.ptr()) == size);
	}

	if ((flags & EfElevated) != 0)
	{
		TCHAR cmd[32768], parameters[32768], cwd[MAX_PATH];

		StringOutputStream ss;
		if (executablePath.getPathName().find(' ') != std::wstring::npos)
			ss << L"\"" << executablePath.getPathName() << L"\"";
		else
			ss << executablePath.getPathName();

		_tcscpy_s(cmd, wstots(ss.str()).c_str());
		_tcscpy_s(parameters, wstots(arguments).c_str());
		_tcscpy_s(cwd, wstots(workingDirectoryAbs.getPathName()).c_str());

		SHELLEXECUTEINFO shExInfo = { 0 };
		shExInfo.cbSize = sizeof(shExInfo);
		shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shExInfo.hwnd = 0;
		shExInfo.lpVerb = _T("runas");
		shExInfo.lpFile = cmd;
		shExInfo.lpParameters = parameters;
		shExInfo.lpDirectory = cwd;
		shExInfo.nShow = SW_SHOW;
		shExInfo.hInstApp = 0;

		if (!ShellExecuteEx(&shExInfo))
			return nullptr;

		return new ProcessShellWin32(shExInfo.hProcess);
	}
	else
	{
		HANDLE hStdInRead = 0, hStdInWrite = 0;
		HANDLE hStdOutRead = 0, hStdOutWrite = 0;
		HANDLE hStdErrRead = 0, hStdErrWrite = 0;
		TCHAR cmd[32768], cwd[MAX_PATH];

		StringOutputStream ss;
		if (executablePath.getPathName().find(' ') != std::wstring::npos)
			ss << L"\"" << executablePath.getPathName() << L"\"";
		else
			ss << executablePath.getPathName();

		if (!arguments.empty())
			ss << L" " << arguments;

		_tcscpy_s(cmd, wstots(ss.str()).c_str());
		_tcscpy_s(cwd, wstots(workingDirectoryAbs.getPathName()).c_str());

		if ((flags & (EfRedirectStdIO | EfMute)) != 0)
		{
			// Create IO objects.
			SECURITY_DESCRIPTOR sd;
			InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
			SetSecurityDescriptorDacl(&sd, true, NULL, false);

			SECURITY_ATTRIBUTES sa;
			std::memset(&sa, 0, sizeof(sa));
			sa.nLength = sizeof(SECURITY_ATTRIBUTES);
			sa.lpSecurityDescriptor = &sd;
			sa.bInheritHandle = TRUE;

			CreatePipe(
				&hStdInRead,
				&hStdInWrite,
				&sa,
				0
			);
			SetHandleInformation(hStdInWrite, HANDLE_FLAG_INHERIT, 0);

			CreatePipe(
				&hStdOutRead,
				&hStdOutWrite,
				&sa,
				0
			);
			SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);

			CreatePipe(
				&hStdErrRead,
				&hStdErrWrite,
				&sa,
				0
			);
			SetHandleInformation(hStdErrRead, HANDLE_FLAG_INHERIT, 0);
		}

		STARTUPINFO si;
		std::memset(&si, 0, sizeof(si));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = ((flags & (EfRedirectStdIO | EfMute)) != 0) ? STARTF_USESTDHANDLES : 0;
		si.hStdInput = hStdInRead;
		si.hStdOutput = hStdOutWrite;
		si.hStdError = hStdErrWrite;

		PROCESS_INFORMATION pi;
		std::memset(&pi, 0, sizeof(pi));

		DWORD dwCreationFlags = CREATE_NEW_PROCESS_GROUP;
		if ((flags & EfMute) != 0)
		{
			dwCreationFlags = CREATE_NO_WINDOW;
			if ((flags & EfDetach) != 0)
				dwCreationFlags |= DETACHED_PROCESS;
		}
		else
			dwCreationFlags = CREATE_NEW_CONSOLE;

		BOOL result = CreateProcess(
			NULL,
			cmd,
			NULL,
			NULL,
			TRUE,
			dwCreationFlags,
			environment.ptr(),
			(cwd[0] != L'\0' ? cwd : NULL),
			&si,
			&pi
		);
		if (result == FALSE)
		{
			T_DEBUG(L"Unable to create process, error = " << (int32_t)GetLastError());
			return nullptr;
		}

		CloseHandle(hStdInRead);
		CloseHandle(hStdOutWrite);
		CloseHandle(hStdErrWrite);

		return new ProcessWin32(
			pi.hProcess,
			pi.dwProcessId,
			pi.hThread,
			hStdInWrite,
			hStdOutRead,
			hStdErrRead
		);
	}
}

Ref< ISharedMemory > OS::createSharedMemory(const std::wstring& name, uint32_t size) const
{
	Ref< SharedMemoryWin32 > sharedMemory = new SharedMemoryWin32();
	if (sharedMemory->create(name, size))
		return sharedMemory;
	else
		return nullptr;
}

bool OS::setOwnProcessPriorityBias(int32_t priorityBias)
{
	bool result = false;
	switch (priorityBias)
	{
	case -1:
		result = bool(SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS) == TRUE);
		break;
	case 0:
		result = bool(SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS) == TRUE);
		break;
	case 1:
		result = bool(SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS) == TRUE);
		break;
	default:
		break;
	}
	return result;
}

bool OS::getRegistry(const std::wstring& key, const std::wstring& subKey, const std::wstring& valueName, std::wstring& outValue) const
{
	HKEY hOpenedKey = NULL;
	HKEY hKey = NULL;

	if (key == L"HKEY_CLASSES_ROOT")
		hKey = HKEY_CLASSES_ROOT;
	else if (key == L"HKEY_CURRENT_USER")
		hKey = HKEY_CURRENT_USER;
	else if (key == L"HKEY_LOCAL_MACHINE")
		hKey = HKEY_LOCAL_MACHINE;
	else if (key == L"HKEY_USERS")
		hKey = HKEY_USERS;
	else if (key == L"HKEY_CURRENT_CONFIG")
		hKey = HKEY_CURRENT_CONFIG;
	else
	{
		log::error << L"Unknown key \"" << key << L"\"" << Endl;
		return false;
	}

	if (RegOpenKeyExW(hKey, !subKey.empty() ? subKey.c_str() : NULL, 0, KEY_READ, &hOpenedKey) != ERROR_SUCCESS)
	{
		log::error << L"RegOpenKeyExW failed" << Endl;
		return false;
	}

	BYTE data[4097];
	DWORD dwDataSize = sizeof(data) - 1;
	DWORD dwType;

	if (RegQueryValueExW(
		hOpenedKey,
		!valueName.empty() ? valueName.c_str() : NULL,
		NULL,
		&dwType,
		data,
		&dwDataSize
	) != ERROR_SUCCESS)
	{
		log::error << L"RegQueryValueExW failed" << Endl;
		RegCloseKey(hOpenedKey);
		return false;
	}

	RegCloseKey(hOpenedKey);
	data[dwDataSize] = 0;

	switch (dwType)
	{
	case REG_DWORD:
		outValue = toString(*(uint32_t*)data);
		return true;

	case REG_SZ:
		outValue = std::wstring((wchar_t*)data);
		return true;

	case REG_EXPAND_SZ:
		outValue = std::wstring((wchar_t*)data);
		return true;

	default:
		log::error << L"Unsupported reg value type." << Endl;
		break;
	}

	return false;
}

bool OS::whereIs(const std::wstring& executable, Path& outPath) const
{
	std::wstring paths;

	// Get system "PATH" environment variable.
	if (!getEnvironment(L"PATH", paths))
		return false;

	// Try to locate binary in any of the paths specified in "PATH".
	for (auto path : StringSplit< std::wstring >(paths, L";,"))
	{
		Ref< File > file = FileSystem::getInstance().get(path + L"/" + executable + L".exe");
		if (file)
		{
			outPath = file->getPath();
			return true;
		}
	}

	return false;
}

OS::OS()
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	// Load IEFrame library; only available on Vista.
	s_hIeFrameLib = LoadLibrary(L"ieframe.dll");
	if (s_hIeFrameLib)
	{
		s_IEIsProtectedModeProcess = (IEISPROTECTEDMODEPROCESSPROC*)GetProcAddress(s_hIeFrameLib, "IEIsProtectedModeProcess");
		T_ASSERT(s_IEIsProtectedModeProcess);
		s_IEGetWriteableFolderPath = (IEGETWRITEABLEFOLDERPATHPROC*)GetProcAddress(s_hIeFrameLib, "IEGetWriteableFolderPath");
		T_ASSERT(s_IEGetWriteableFolderPath);
	}
}

OS::~OS()
{
	if (s_hIeFrameLib)
		FreeLibrary(s_hIeFrameLib);

	CoUninitialize();
}

void OS::destroy()
{
	T_SAFE_RELEASE(this);
}

}
