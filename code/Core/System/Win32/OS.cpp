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
#include "Core/Misc/TString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/System/OS.h"
#include "Core/System/ResolveEnv.h"
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.OS", OS, Object)

OS& OS::getInstance()
{
	static OS* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new OS();
		s_instance->addRef(0);
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
#if !defined(WINCE)
	wchar_t buf[MAX_COMPUTERNAME_LENGTH + 1];

	DWORD size = sizeof_array(buf);
	if (!GetComputerName(buf, &size))
		return L"";

	return buf;
#else
	return L"";
#endif
}

std::wstring OS::getCurrentUser() const
{
#if !defined(WINCE)
	wchar_t buf[UNLEN + 1];
	
	DWORD size = sizeof_array(buf);
	if (!GetUserName(buf, &size))
		return L"";

	return buf;
#else
	return L"";
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

bool OS::openFile(const std::wstring& file) const
{
#if !defined(WINCE)
	HINSTANCE hInstance = ShellExecute(
		NULL,
		_T("open"),
		wstots(file).c_str(),
		NULL,
		NULL,
		SW_SHOWDEFAULT
	);
	return int(hInstance) > 32;
#else
	return false;
#endif
}

bool OS::editFile(const std::wstring& file) const
{
#if !defined(WINCE)
	HINSTANCE hInstance = ShellExecute(
		NULL,
		_T("edit"),
		wstots(file).c_str(),
		NULL,
		NULL,
		SW_SHOWDEFAULT
	);
	return int(hInstance) > 32;
#else
	return false;
#endif
}

bool OS::exploreFile(const std::wstring& file) const
{
#if !defined(WINCE)
	HINSTANCE hInstance = ShellExecute(
		NULL,
		_T("explore"),
		wstots(file).c_str(),
		NULL,
		NULL,
		SW_SHOWDEFAULT
	);
	return int(hInstance) > 32;
#else
	return false;
#endif
}

OS::envmap_t OS::getEnvironment() const
{
	envmap_t envmap;

#if !defined(WINCE)
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
				envmap.insert(std::make_pair(
					tstows(tstring(key, sep)),
					tstows(val)
				));
			}

			p = val + _tcslen(val) + 1;
		}
	}
#endif

	return envmap;
}

bool OS::getEnvironment(const std::wstring& name, std::wstring& outValue) const
{
#if !defined(WINCE)
	const char* env = getenv(wstombs(name).c_str());
	if (!env)
		return false;

	outValue = mbstows(env);
	return true;
#else
	return false;
#endif
}

Ref< IProcess > OS::execute(
	const std::wstring& commandLine,
	const Path& workingDirectory,
	const envmap_t* envmap,
	bool redirect,
	bool mute,
	bool detach
) const
{
	TCHAR cmd[32768], par[1024], cwd[MAX_PATH];
	HANDLE hStdInRead = 0, hStdInWrite = 0;
	HANDLE hStdOutRead = 0, hStdOutWrite = 0;
	HANDLE hStdErrRead = 0, hStdErrWrite = 0;
	AutoArrayPtr< wchar_t > environment;
	std::wstring executable;
	std::wstring arguments;

	// Resolve entire command line.
	std::wstring resolvedCommandLine = resolveEnv(commandLine);

	// Extract executable file from command line.
	if (resolvedCommandLine.empty())
		return 0;

	if (resolvedCommandLine[0] == L'\"')
	{
		size_t i = resolvedCommandLine.find(L'\"', 1);
		if (i == resolvedCommandLine.npos)
			return 0;

		executable = resolvedCommandLine.substr(1, i - 1);
		arguments = resolvedCommandLine.substr(i + 1);
	}
	else
	{
		size_t i = resolvedCommandLine.find(L' ');
		if (i != resolvedCommandLine.npos)
		{
			executable = resolvedCommandLine.substr(0, i);
			arguments = resolvedCommandLine.substr(i + 1);
		}
		else
			executable = resolvedCommandLine;
	}

	// Resolve absolute paths.
	Path fileAbs = FileSystem::getInstance().getAbsolutePath(executable);
	Path workingDirectoryAbs = FileSystem::getInstance().getAbsolutePath(workingDirectory);

	// Create environment variables.
	if (envmap)
	{
		// Calculate how much space we need to allocate.
		uint32_t size = 0;
		for (envmap_t::const_iterator i = envmap->begin(); i != envmap->end(); ++i)
			size += i->first.length() + 1 + i->second.length() + 1;
		size += 1;

		environment.reset(new wchar_t [size]);

		wchar_t* p = environment.ptr(); *p = 0;
		for (envmap_t::const_iterator i = envmap->begin(); i != envmap->end(); ++i)
		{
			wcscpy(p, i->first.c_str());
			wcscat(p, L"=");
			wcscat(p, i->second.c_str());
			p += i->first.length() + 1 + i->second.length() + 1;
		}
		*p++ = 0;

		T_ASSERT (size_t(p - environment.ptr()) == size);
	}

	StringOutputStream ss;
	ss << L"\"" << fileAbs.getPathName() << L"\"";
	if (!arguments.empty())
		ss << L" " << arguments;

#if !defined(WINCE)
	_tcscpy_s(cmd, wstots(ss.str()).c_str());
	_tcscpy_s(cwd, wstots(workingDirectoryAbs.getPathName()).c_str());
#else
	_tcscpy_s(cmd, sizeof_array(cmd), wstots(ss.str()).c_str());
	_tcscpy_s(cwd, sizeof_array(cwd), wstots(workingDirectoryAbs.getPathName()).c_str());
#endif

#if !defined(WINCE)
	if (redirect)
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

#if !defined(WINCE)
	DWORD dwCreationFlags = CREATE_NEW_PROCESS_GROUP;

	if (mute)
	{
		dwCreationFlags = CREATE_NO_WINDOW;
		if (detach)
			dwCreationFlags |= DETACHED_PROCESS;
	}
	else
		dwCreationFlags = CREATE_NEW_CONSOLE;

	if (environment.ptr())
		dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
#else
	DWORD dwCreationFlags = mute ? 0 : CREATE_NEW_CONSOLE;
#endif

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
		return 0;
	}

	return new ProcessWin32(
		pi.hProcess,
		pi.dwProcessId,
		pi.hThread,
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

bool OS::setOwnProcessPriorityBias(int32_t priorityBias)
{
	bool result = false;
#if !defined(WINCE)
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
#endif
	return result;
}

OS::OS()
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

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
	T_SAFE_RELEASE(this);
}

}
