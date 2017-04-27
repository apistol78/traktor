/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Game/App/Win32/ErrorDialog.h"
#include "Amalgam/Game/App/Win32/StackWalker.h"
#include "Amalgam/Game/Impl/Application.h"
#include "Core/Date/DateTime.h"
#include "Core/Io/FileOutputStreamBuffer.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/System/OS.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Semaphore.h"
#include "Ui/Application.h"
#include "Ui/Win32/EventLoopWin32.h"
#include "Ui/Win32/WidgetFactoryWin32.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

using namespace traktor;

// NVidia hack to get Optimus to enable NVidia GPU when possible.
extern "C"
{
    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

namespace
{

class StackWalkerToConsole : public StackWalker
{
protected:
	// Overload to get less output by stackwalker.
	virtual void OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName) T_OVERRIDE T_FINAL {}	
	virtual void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr) T_OVERRIDE T_FINAL {}
	virtual void OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion) T_OVERRIDE T_FINAL {}

	virtual void OnOutput(LPCSTR szText) T_OVERRIDE T_FINAL
	{
		log::info << mbstows(szText);
	}
};

class LogTailTarget : public ILogTarget
{
public:
	Semaphore m_lock;
	std::list< std::wstring > m_tail;

	virtual void log(uint32_t threadId, int32_t level, const std::wstring& str) T_OVERRIDE T_FINAL
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		if (m_tail.size() > 100)
			m_tail.pop_front();
		m_tail.push_back(str);
	}
};

class LogStreamTarget : public ILogTarget
{
public:
	LogStreamTarget(OutputStream* stream)
	:	m_stream(stream)
	{
	}

	virtual void log(uint32_t threadId, int32_t level, const std::wstring& str) T_OVERRIDE T_FINAL
	{
		(*m_stream) << L"[" << DateTime::now().format(L"%H:%M:%S") << L"] " << str << Endl;
	}

private:
	Ref< OutputStream > m_stream;
};

class LogDualTarget : public ILogTarget
{
public:
	LogDualTarget(ILogTarget* target1, ILogTarget* target2)
	:	m_target1(target1)
	,	m_target2(target2)
	{
	}

	virtual void log(uint32_t threadId, int32_t level, const std::wstring& str) T_OVERRIDE T_FINAL
	{
		m_target1->log(threadId, level, str);
		m_target2->log(threadId, level, str);
	}

private:
	Ref< ILogTarget > m_target1;
	Ref< ILogTarget > m_target2;
};

Ref< PropertyGroup > loadSettings(const Path& settingsFile)
{
	Ref< PropertyGroup > settings;

	Ref< traktor::IStream > file = FileSystem::getInstance().open(settingsFile, File::FmRead);
	if (file)
	{
		settings = xml::XmlDeserializer(file).readObject< PropertyGroup >();
		file->close();
	}

	return settings;
}

bool saveSettings(const PropertyGroup* settings, const Path& settingsFile)
{
	T_ASSERT (settings);

	Ref< traktor::IStream > file = FileSystem::getInstance().open(settingsFile, File::FmWrite);
	if (!file)
		return false;

	bool result = xml::XmlSerializer(file).writeObject(settings);
	file->close();
	
	return result;
}

void showErrorDialog(const std::list< std::wstring >& tail)
{
	amalgam::ErrorDialog errorDialog;
	if (errorDialog.create())
	{
		for (std::list< std::wstring >::const_iterator i = tail.begin(); i != tail.end(); ++i)
			errorDialog.addErrorString(*i);
			
		errorDialog.addErrorString(L"Please copy this information and contact");
		errorDialog.addErrorString(L"support@doctorentertainment.com");

		errorDialog.showModal();
		errorDialog.destroy();
	}
}

bool checkPreconditions()
{
	BOOL sseSupported = ::IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE);
	if (!sseSupported)
	{
		log::error << L"This game requires a CPU with SSE support" << Endl;
		return false;
	}
	return true;
}

bool isWindows64bit(bool& out64bit)
{
	out64bit = false;
#if _WIN64
    out64bit =  true;
    return true;
#elif _WIN32
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
    if(fnIsWow64Process)
    {
		BOOL isWow64 = FALSE;
        if (!fnIsWow64Process(GetCurrentProcess(), &isWow64))
            return false;

        if(isWow64)
            out64bit = true;
        else
            out64bit = false;

        return true;
    }
    else
        return false;
#else
    return false;
#endif
}

void logSystemInfo()
{
	// Log CPU info
	char CPUString[0x20];
	int cpuInfo[4] = {-1};

	__cpuid(cpuInfo, 0);
	int nIds = cpuInfo[0];
	std::memset(CPUString, 0, sizeof(CPUString));
	*((int*)CPUString) = cpuInfo[1];
	*((int*)(CPUString+4)) = cpuInfo[3];
	*((int*)(CPUString+8)) = cpuInfo[2];
	log::info << L"CPU" << Endl;
	log::info << L"\tString \"" << trim(mbstows(CPUString)) << L"\""  << Endl;

	if  (nIds >= 1) 
	{
		__cpuid(cpuInfo, 1);
		log::info << L"\tFamily " << ((cpuInfo[0] >> 8)  & 0xf) << Endl;
		log::info << L"\tModel " << ((cpuInfo[0] >> 4) & 0xf) << Endl;
		log::info << L"\tStepping " << (cpuInfo[0] & 0xf) << Endl;
	}
	if  (nIds >= 4) 
	{
		// Get number of cores
		__cpuidex(cpuInfo, 0x4, 0);
		log::info << L"\tCores (cpuinfo) " << ((cpuInfo[0] >> 26) + 1) << Endl;
	}

	log::info << L"\tCores (Win API) " << OS::getInstance().getCPUCoreCount() << Endl;

	// Calling __cpuid with 0x80000000 as the function_id argument
	// gets the number of valid extended IDs.
	__cpuid(cpuInfo, 0x80000000);
	int nExIds = cpuInfo[0];

	char CPUBrandString[0x40];
	std::memset(CPUBrandString, 0, sizeof(CPUBrandString));
	for (int i=0x80000000; i<=nExIds; ++i)
	{
		__cpuid(cpuInfo, i);
		if  (i == 0x80000002)
			memcpy(CPUBrandString, cpuInfo, sizeof(cpuInfo));
		else if  (i == 0x80000003)
			memcpy(CPUBrandString + 16, cpuInfo, sizeof(cpuInfo));
		else if  (i == 0x80000004)
			memcpy(CPUBrandString + 32, cpuInfo, sizeof(cpuInfo));
	}
	if  (nExIds >= 0x80000004)
		log::info << L"\tBrand String \"" << trim(mbstows(CPUBrandString)) << L"\"" << Endl;

	// Log memory info
	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	log::info << L"System Memory" << Endl;
	log::info << L"\tTotal physical " << memInfo.ullTotalPhys / 1024 / 1024 << L" MiB" << Endl;
	log::info << L"\tAvailable physical " << memInfo.ullAvailPhys  / 1024 / 1024  << L" MiB" << Endl;
	log::info << L"\tTotal virtual " << memInfo.ullTotalVirtual  / 1024 / 1024  << L" MiB" << Endl;
	log::info << L"\tAvailable virtual " << memInfo.ullAvailVirtual  / 1024 / 1024  << L" MiB" << Endl;

	// Log OS Version
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO *) &osvi);
	log::info << L"Operating System" << Endl;
	if (osvi.wProductType == VER_NT_WORKSTATION) 
	{
		if (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0)
			log::info << L"\tWindows 10";
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3)
			log::info << L"\tWindows 8.1";
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2)
			log::info << L"\tWindows 8";
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
			log::info << L"\tWindows 7";
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
			log::info << L"\tWindows Vista";
	}
	else
	{
		if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3)
			log::info << L"\tWindows Server 2012 R2";
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2)
			log::info << L"\tWindows Server 2012";
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
			log::info << L"\tWindows Server 2008 R2";
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
			log::info << L"\tWindows Server 2008";		
	}

	bool is64bit = false;
	if (isWindows64bit(is64bit))
		log::info << L" (" << (is64bit ? L"64-bit" : L"32-bit") << L")";

	if (osvi.szCSDVersion)
		log::info << L" " << osvi.szCSDVersion << Endl;
	else 
		log::info << Endl;
	log::info << L"\tOS version " << uint32_t(osvi.dwMajorVersion) << L"." << uint32_t(osvi.dwMinorVersion) << L"." << uint32_t(osvi.dwBuildNumber) << Endl;
	log::info << L"\tSP version " << uint32_t(osvi.wServicePackMajor) << L"." << uint32_t(osvi.wServicePackMinor) << Endl;
}

std::wstring getExceptionString(DWORD exceptionCode)
{
	switch (exceptionCode)
	{
		case EXCEPTION_ACCESS_VIOLATION:		return L"EXCEPTION_ACCESS_VIOLATION";
		case EXCEPTION_DATATYPE_MISALIGNMENT:	return L"EXCEPTION_DATATYPE_MISALIGNMENT";
		case EXCEPTION_BREAKPOINT:				return L"EXCEPTION_BREAKPOINT";
		case EXCEPTION_SINGLE_STEP:				return L"EXCEPTION_SINGLE_STEP";
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:	return L"EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
		case EXCEPTION_FLT_DENORMAL_OPERAND:	return L"EXCEPTION_FLT_DENORMAL_OPERAND";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:		return L"EXCEPTION_FLT_DIVIDE_BY_ZERO";
		case EXCEPTION_FLT_INEXACT_RESULT:		return L"EXCEPTION_FLT_INEXACT_RESULT";
		case EXCEPTION_FLT_INVALID_OPERATION:	return L"EXCEPTION_FLT_INVALID_OPERATION";
		case EXCEPTION_FLT_OVERFLOW:			return L"EXCEPTION_FLT_OVERFLOW";
		case EXCEPTION_FLT_STACK_CHECK:			return L"EXCEPTION_FLT_STACK_CHECK";
		case EXCEPTION_FLT_UNDERFLOW:			return L"EXCEPTION_FLT_UNDERFLOW";
		case EXCEPTION_INT_DIVIDE_BY_ZERO:		return L"EXCEPTION_INT_DIVIDE_BY_ZERO";
		case EXCEPTION_INT_OVERFLOW:			return L"EXCEPTION_INT_OVERFLOW";
		case EXCEPTION_PRIV_INSTRUCTION:		return L"EXCEPTION_PRIV_INSTRUCTION";
		case EXCEPTION_IN_PAGE_ERROR:			return L"EXCEPTION_IN_PAGE_ERROR";
		case EXCEPTION_ILLEGAL_INSTRUCTION:		return L"EXCEPTION_ILLEGAL_INSTRUCTION";
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:return L"EXCEPTION_NONCONTINUABLE_EXCEPTION";
		case EXCEPTION_STACK_OVERFLOW:			return L"EXCEPTION_STACK_OVERFLOW";
		case EXCEPTION_INVALID_DISPOSITION:		return L"EXCEPTION_INVALID_DISPOSITION";
		case EXCEPTION_GUARD_PAGE:				return L"EXCEPTION_GUARD_PAGE";
		default:								return L"UNKNOWN EXCEPTION";					
	}
}

void* g_exceptionAddress = 0;
DWORD g_exceptionCode = 0;

LONG WINAPI exceptionVectoredHandler(struct _EXCEPTION_POINTERS* ep)
{
	g_exceptionAddress = (void*)ep->ExceptionRecord->ExceptionAddress;
	g_exceptionCode = ep->ExceptionRecord->ExceptionCode;

	bool outputCallStack = false;
	bool outputInfo = false;
	switch (ep->ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:		
	case EXCEPTION_DATATYPE_MISALIGNMENT:	
	case EXCEPTION_STACK_OVERFLOW:			
	case EXCEPTION_ILLEGAL_INSTRUCTION:		
	case EXCEPTION_PRIV_INSTRUCTION:		
	case EXCEPTION_IN_PAGE_ERROR:			
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
	case EXCEPTION_INVALID_DISPOSITION:		
	case EXCEPTION_GUARD_PAGE:				
//	case EXCEPTION_POSSIBLE_DEADLOCK:
	case EXCEPTION_INVALID_HANDLE:
		outputCallStack = true;
		break;

	case EXCEPTION_BREAKPOINT:				
	case EXCEPTION_SINGLE_STEP:				
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:	
	case EXCEPTION_FLT_DENORMAL_OPERAND:	
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:		
	case EXCEPTION_FLT_INEXACT_RESULT:		
	case EXCEPTION_FLT_INVALID_OPERATION:	
	case EXCEPTION_FLT_OVERFLOW:			
	case EXCEPTION_FLT_STACK_CHECK:			
	case EXCEPTION_FLT_UNDERFLOW:			
	case EXCEPTION_INT_DIVIDE_BY_ZERO:		
	case EXCEPTION_INT_OVERFLOW:			
		outputInfo = true;
		break;

	default:								
		break;
	}

	if (outputCallStack)
	{
		log::info << L"Thread " << (uint32_t)GetCurrentThread() << L":" << Endl;
		log::info << IncreaseIndent;

		StackWalkerToConsole sw;
		sw.ShowCallstack(GetCurrentThread(), ep->ContextRecord);

		log::info << DecreaseIndent;
	}
	else if (outputInfo)
	{
		HMODULE hCrashModule;
		if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCTSTR>(g_exceptionAddress), &hCrashModule))
		{
			TCHAR fileName[MAX_PATH];
			GetModuleFileName(hCrashModule, fileName, sizeof_array(fileName));
			log::debug << L"Exception ( " << getExceptionString(g_exceptionCode) << L") occurred at 0x" << g_exceptionAddress << L" in module " << (void*)hCrashModule << L" " << fileName << Endl;
		}
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

void pureVirtualCallHandler(void)
{
	CONTEXT c;
	GET_CURRENT_CONTEXT(c, CONTEXT_FULL);
	StackWalkerToConsole sw;
	sw.ShowCallstack(GetCurrentThread(), &c);
	exit(0);
}

}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR szCmdLine, int)
{
	std::vector< std::wstring > argv;
	SystemApplication sysapp;

	wchar_t szFilename[MAX_PATH] = L"";
	GetModuleFileName(NULL, szFilename, sizeof_array(szFilename));
	argv.push_back(szFilename);

	Split< std::wstring >::any(mbstows(szCmdLine), L" \t", argv);
	CommandLine cmdLine(argv);
	Ref< traktor::IStream > logFile;

	if (!checkPreconditions())
		return 1;

	std::wstring writablePath = OS::getInstance().getWritableFolderPath() + L"/Doctor Entertainment AB";
	FileSystem::getInstance().makeAllDirectories(writablePath);

#if !defined(_DEBUG)
	if (!IsDebuggerPresent())
	{
		RefArray< File > logs;
		FileSystem::getInstance().find(L"Application_*.log", logs);

		// Get "alive" log ids.
		std::vector< int32_t > logIds;
		for (RefArray< File >::const_iterator i = logs.begin(); i != logs.end(); ++i)
		{
			std::wstring logName = (*i)->getPath().getFileNameNoExtension();
			size_t p = logName.find(L'_');
			if (p != logName.npos)
			{
				int32_t id = parseString< int32_t >(logName.substr(p + 1), -1);
				if (id != -1)
					logIds.push_back(id);
			}
		}

		int32_t nextLogId = 0;
		if (!logIds.empty())
		{
			std::sort(logIds.begin(), logIds.end());

			// Don't keep more than 10 log files.
			while (logIds.size() >= 10)
			{
				StringOutputStream ss;
				ss << L"Application_" << logIds.front() << L".log";
				FileSystem::getInstance().remove(ss.str());
				logIds.erase(logIds.begin());
			}

			nextLogId = logIds.back() + 1;
		}

		// Create new log file.
		StringOutputStream ss;
		ss << L"Application_" << nextLogId << L".log";
		logFile = FileSystem::getInstance().open(ss.str(), File::FmWrite);
		if (logFile)
		{
			Ref< FileOutputStream > logStream = new FileOutputStream(logFile, new Utf8Encoding());
			Ref< LogStreamTarget > logTarget = new LogStreamTarget(logStream);

			log::info   .setGlobalTarget(logTarget);
			log::warning.setGlobalTarget(logTarget);
			log::error  .setGlobalTarget(logTarget);

			log::info << L"Log file \"Application.log\" created" << Endl;
		}
		else
			log::error << L"Unable to create log file; logging only to std pipes" << Endl;
	}
#endif

	logSystemInfo();

	Ref< LogTailTarget > logTail = new LogTailTarget();
	log::info   .setGlobalTarget(new LogDualTarget(logTail, log::info   .getGlobalTarget()));
	log::warning.setGlobalTarget(new LogDualTarget(logTail, log::warning.getGlobalTarget()));
	log::error  .setGlobalTarget(new LogDualTarget(logTail, log::error  .getGlobalTarget()));

#if defined(_WIN64)
	log::info << L"Application starting (64-bit) ..." << Endl;
#else
	log::info << L"Application starting (32-bit) ..." << Endl;
#endif

#if !defined(_WIN64)
	// Ensure FP is in known state.
	_controlfp(_PC_24, _MCW_PC);
	_controlfp(_RC_NEAR, _MCW_RC);
#endif

	// Initialize native UI.
	ui::Application::getInstance()->initialize(
		new ui::EventLoopWin32(),
		new ui::WidgetFactoryWin32(),
		0
	);

	Ref< amalgam::Application > application;

#if !defined(_DEBUG)
	try
#endif
	{
#if !defined(_DEBUG)
		SetErrorMode(SEM_NOGPFAULTERRORBOX);
		PVOID eh = AddVectoredExceptionHandler(1, exceptionVectoredHandler);
#endif
		_set_purecall_handler(pureVirtualCallHandler);

		Path currentPath = FileSystem::getInstance().getAbsolutePath(L".");
		log::info << L"Working directory: " <<currentPath.getPathName() << Endl;

		// Override settings path either from command line or application bundle.
		Path settingsPath = L"Application.config";
		if (cmdLine.getCount() >= 1)
			settingsPath = cmdLine.getString(0);

		Ref< PropertyGroup > defaultSettings = loadSettings(settingsPath);
		if (!defaultSettings)
		{
			// Steam bug fix #1) Apparently there are sometimes issues with current working directory when launched from Steam.
			defaultSettings = loadSettings(Path(L"..") + settingsPath);
			if (!defaultSettings)
			{
				log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L")." << Endl;
				log::error << L"Please reinstall application." << Endl;
				showErrorDialog(logTail->m_tail);
				return 1;
			}
			else
			{
				// Application is started in "bin" directory; change cwd and continue.
				Path cd = FileSystem::getInstance().getCurrentVolume()->getCurrentDirectory();
				FileSystem::getInstance().getCurrentVolume()->setCurrentDirectory((cd + Path(L"..")).normalized());
			}
		}

		Ref< PropertyGroup > settings = DeepClone(defaultSettings).create< PropertyGroup >();
		T_FATAL_ASSERT (settings);

		// Merge user settings into application settings.
		if (!cmdLine.hasOption('s', L"no-settings"))
		{
			Path userSettingsPath;
			Ref< PropertyGroup > userSettings;

			// First try to load user settings from current working directory; ie. same directory as
			// the main executable.
			userSettingsPath = settingsPath.getPathNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + settingsPath.getExtension();
			userSettings = loadSettings(userSettingsPath);
	
			// Try to load user settings from user's application data path; sometimes it's not possible
			// to store user settings alongside executable due to restrictive privileges.
			if (!userSettings)
			{
				userSettingsPath = writablePath + L"/" + settingsPath.getFileNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + settingsPath.getExtension();
				userSettings = loadSettings(userSettingsPath);
			}

			if (userSettings)
				settings = settings->mergeReplace(userSettings);
		}

		if (!settings)
		{
			log::error << L"Unable to read application settings (" << settingsPath.getPathName() << L")." << Endl;
			log::error << L"Please reinstall application." << Endl;
			showErrorDialog(logTail->m_tail);
			return 1;
		}

		// Override settings from command line.
		if (cmdLine.hasOption(L"force-render-device"))
		{
			int32_t renderDevice = cmdLine.getOption(L"force-render-device").getInteger();
			if (renderDevice >= 0)
				settings->setProperty< PropertyInteger >(L"Render.Adapter", renderDevice);
		}
		if (cmdLine.hasOption(L"force-language-code"))
		{
			std::wstring languageCode = cmdLine.getOption(L"force-language-code").getString();
			if (!languageCode.empty())
				settings->setProperty< PropertyString >(L"Online.OverrideLanguageCode", languageCode);
		}

		// Create amalgam application.
		application = new amalgam::Application();
		if (application->create(
			defaultSettings,
			settings,
			sysapp,
			0
		))
		{
			for (;;)
			{
				if (!application->update())
					break;
			}

			safeDestroy(application);

			// Save user settings.
			if (!cmdLine.hasOption('s', L"no-settings"))
			{
				Path userSettingsPath = settingsPath.getPathNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + settingsPath.getExtension();
				if (!saveSettings(settings, userSettingsPath))
				{
					userSettingsPath = writablePath + L"/" + settingsPath.getFileNameNoExtension() + L"." + OS::getInstance().getCurrentUser() + L"." + settingsPath.getExtension();
					if (!saveSettings(settings, userSettingsPath))
						log::error << L"Unable to save user settings; user changes not saved" << Endl;
				}
			}
		}
		else
		{
			safeDestroy(application);
			showErrorDialog(logTail->m_tail);
		}

#if !defined(_DEBUG)
		RemoveVectoredExceptionHandler(eh);
#endif
	}
#if !defined(_DEBUG)
	catch (...)
	{
		HMODULE hCrashModule;
		if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCTSTR>(g_exceptionAddress), &hCrashModule))
		{
			TCHAR fileName[MAX_PATH];
			GetModuleFileName(hCrashModule, fileName, sizeof_array(fileName));
			log::error << L"Unhandled exception ( " << getExceptionString(g_exceptionCode) << L") occurred at 0x" << g_exceptionAddress << L" in module " << (void*)hCrashModule << L" " << fileName << Endl;
		}
		else
			log::error << L"Unhandled exception ( " << getExceptionString(g_exceptionCode) << L") occurred at 0x" << g_exceptionAddress << Endl;

		safeDestroy(application);
		showErrorDialog(logTail->m_tail);
	}
#endif

	ui::Application::getInstance()->finalize();

#if !defined(_DEBUG)
	if (logFile)
	{
		logFile->close();
		logFile;
	}
#endif

	log::info   .setGlobalTarget(0);
	log::warning.setGlobalTarget(0);
	log::error  .setGlobalTarget(0);

#if defined(_DEBUG)
	SingletonManager::getInstance().destroy();
#endif
	return 0;
}
