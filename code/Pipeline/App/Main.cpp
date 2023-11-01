/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <iostream>
#include "Core/Io/FileSystem.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Io/Writer.h"
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Log/LogRedirectTarget.h"
#include "Core/Log/LogStreamTarget.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/System/OS.h"
#include "Core/Thread/Mutex.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/IEvent.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Database/Compact/CompactDatabase.h"
#include "Database/Events/EvtInstanceCommitted.h"
#include "Database/Events/EvtInstanceCreated.h"
#include "Database/Events/EvtInstanceRemoved.h"
#include "Database/Local/LocalDatabase.h"
#include "Editor/Assets.h"
#include "Editor/IPipeline.h"
#include "Editor/Pipeline/PipelineBuilder.h"
#include "Editor/Pipeline/PipelineDbFlat.h"
#include "Editor/PipelineDependencySet.h"
#include "Editor/Pipeline/PipelineDependsIncremental.h"
#include "Editor/Pipeline/PipelineDependsParallel.h"
#include "Editor/Pipeline/PipelineFactory.h"
#include "Editor/Pipeline/PipelineInstanceCache.h"
#include "Editor/Pipeline/PipelineSettings.h"
#include "Editor/Pipeline/Avalanche/AvalanchePipelineCache.h"
#include "Editor/Pipeline/File/FilePipelineCache.h"
#include "Pipeline/App/PipelineParameters.h"
#include "Xml/XmlDeserializer.h"

#if defined(_WIN32)
#	include <windows.h>
#	include <tlhelp32.h>
#	include "Pipeline/App/Win32/StackWalker.h"
#endif

#if defined(__LINUX__) || defined(__RPI__)
#	include <execinfo.h>
#	include <signal.h>
#	include <stdlib.h>
#	include <unistd.h>
#endif

using namespace traktor;

#if defined(_WIN32)

class StackWalkerToConsole : public StackWalker
{
protected:
	// Overload to get less output by stackwalker.
	virtual void OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName) override final {}
	virtual void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr) override final {}
	virtual void OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion) override final {}

	virtual void OnOutput(LPCSTR szText) override final
	{
		log::info << mbstows(szText);
	}
};

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

void* g_exceptionAddress = nullptr;
LONG WINAPI exceptionVectoredHandler(struct _EXCEPTION_POINTERS* ep)
{
	g_exceptionAddress = (void*)ep->ExceptionRecord->ExceptionAddress;
	bool ouputCallStack = true;

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
		ouputCallStack = true;
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
	default:
		ouputCallStack = false;
		break;
	}

	if (ouputCallStack)
	{
		StackWalkerToConsole sw;
		sw.ShowCallstack(GetCurrentThread(), ep->ContextRecord);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

#endif

#if defined(__LINUX__) || defined(__RPI__)

void signalExceptionHandler(int sig)
{
	void* array[10];
	size_t size = backtrace(array, 10);
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);	
}

#endif

struct StatusListener : public editor::IPipelineBuilder::IListener
{
	virtual void beginBuild(
		int32_t index,
		int32_t count,
		const editor::PipelineDependency* dependency
	) override final
	{
		log::info << L":" << index << L":" << count << Endl;
	}

	virtual void endBuild(
		int32_t index,
		int32_t count,
		const editor::PipelineDependency* dependency,
		editor::IPipelineBuilder::BuildResult result
	) override final
	{
	}
};

Ref< PropertyGroup > loadSettings(const std::wstring& settingsFile)
{
	Ref< PropertyGroup > settings;
	Ref< traktor::IStream > file;

#if defined(_WIN32)
	std::wstring system = L"win32";
#elif defined(__APPLE__)
	std::wstring system = L"osx";
#else   // LINUX
	std::wstring system = L"linux";
#endif

	std::wstring globalConfig = settingsFile + L".config";
	std::wstring systemConfig = settingsFile + L"." + system + L".config";
	std::wstring userConfig = settingsFile + L"." + OS::getInstance().getCurrentUser() + L".config";

	// First try to read user configuration; contain both global and system.
	if ((file = FileSystem::getInstance().open(userConfig, File::FmRead)) != 0)
	{
		settings = xml::XmlDeserializer(file, userConfig).readObject< PropertyGroup >();
		if (!settings)
			log::warning << userConfig << L" corrupt!" << Endl;
		file->close();
	}
	if (settings)
		return settings;

	// No user configuration; load global configuration.
	Ref< PropertyGroup > globalSettings;
	Ref< PropertyGroup > systemSettings;

	if ((file = FileSystem::getInstance().open(globalConfig, File::FmRead)) != 0)
	{
		globalSettings = xml::XmlDeserializer(file, globalConfig).readObject< PropertyGroup >();
		if (!globalSettings)
			log::warning << globalConfig << L" corrupt!" << Endl;
		file->close();
	}

	// Read system properties.
	if ((file = FileSystem::getInstance().open(systemConfig, File::FmRead)) != 0)
	{
		systemSettings = xml::XmlDeserializer(file, systemConfig).readObject< PropertyGroup >();
		if (!systemSettings)
			log::warning << systemConfig << L" corrupt!" << Endl;
		file->close();
	}

	// Merge in system configuration.
	if (globalSettings && systemSettings)
		settings = globalSettings->merge(systemSettings, PropertyGroup::MmReplace);
	else
		settings = globalSettings;

	return settings;
}

struct ConnectionAndCache
{
	Ref< db::Database > database;
	Ref< editor::PipelineInstanceCache > cache;
};

bool g_receivedBreakSignal = false;
bool g_success = false;

#if defined(_WIN32)

BOOL consoleCtrlHandler(DWORD fdwCtrlType)
{
	g_receivedBreakSignal = true;
	return TRUE;
}

#endif

void threadBuild(editor::PipelineBuilder& pipelineBuilder, const editor::PipelineDependencySet* dependencySet, bool rebuild)
{
	g_success = pipelineBuilder.build(dependencySet, rebuild);
}

ConnectionAndCache openDatabase(const PropertyGroup* settings, const std::wstring& connectionString, bool create)
{
	log::info << L"Opening database \"" << connectionString << L"\"..." << Endl;

	// Open or create new database.
	Ref< db::Database > database = new db::Database();
	if (!database->open(connectionString))
	{
		if (!create || !database->create(connectionString))
			return ConnectionAndCache();
	}

	// Also create an instance cache.
	Ref< editor::PipelineInstanceCache > cache;
	if (!create)
	{
		std::wstring cachePath = settings->getProperty< std::wstring >(L"Pipeline.InstanceCache.Path");
		cache = new editor::PipelineInstanceCache(database, cachePath);
	}

	return { database, cache };
}

bool perform(const PipelineParameters& params)
{
	if (!FileSystem::getInstance().setCurrentVolumeAndDirectory(params.getWorkingDirectory()))
	{
		traktor::log::error << L"Unable to change working directory." << Endl;
		return false;
	}

	const bool verbose = params.getVerbose();
	if (verbose)
		traktor::log::info << L"Using settings \"" << params.getSettings() << L"\"." << Endl;

	Ref< PropertyGroup > settings = loadSettings(params.getSettings());
	if (!settings)
	{
		traktor::log::error << L"Unable to load pipeline settings \"" << params.getSettings() << L"\"." << Endl;
		return false;
	}

	if (params.getNoCache())
	{
		settings->setProperty< PropertyBoolean >(L"Pipeline.AvalancheCache", false);
		settings->setProperty< PropertyBoolean >(L"Pipeline.FileCache", false);
	}

	if (verbose)
		settings->setProperty< PropertyBoolean >(L"Pipeline.Verbose", true);

	// Load necessary modules.
	auto modulePaths = settings->getProperty< SmallSet< std::wstring > >(L"Editor.ModulePaths");
	auto modules = settings->getProperty< SmallSet< std::wstring > >(L"Editor.Modules");

	std::vector< Path > modulePathsFlatten(modulePaths.begin(), modulePaths.end());
	for (const auto& module : modules)
	{
		Library library;
		if (!library.open(module, modulePathsFlatten, true))
		{
			traktor::log::error << L"Unable to load module \"" << module << L"\"." << Endl;
			return false;
		}
		library.detach();
	}

	// Open database connections.
	std::wstring sourceDatabaseCS = settings->getProperty< std::wstring >(L"Editor.SourceDatabase");
	std::wstring outputDatabaseCS = settings->getProperty< std::wstring >(L"Editor.OutputDatabase");

	ConnectionAndCache sourceDatabaseAndCache = openDatabase(settings, sourceDatabaseCS, false);
	if (!sourceDatabaseAndCache.database)
	{
		traktor::log::error << L"Unable to open source database \"" << sourceDatabaseCS << L"\"." << Endl;
		return false;
	}

	ConnectionAndCache outputDatabaseAndCache = openDatabase(settings, outputDatabaseCS, true);
	if (!outputDatabaseAndCache.database)
	{
		traktor::log::error << L"Unable to open or create output database \"" << outputDatabaseCS << L"\"." << Endl;
		return false;
	}

	// Open pipeline dependency database.
	std::wstring connectionString = settings->getProperty< std::wstring >(L"Pipeline.Db");

	Ref< editor::IPipelineDb > pipelineDb = new editor::PipelineDbFlat();
	if (!pipelineDb->open(connectionString))
	{
		traktor::log::error << L"Unable to connect to pipeline database using connection string \"" << connectionString << L"\"." << Endl;
		return false;
	}

	// Create cache if enabled.
	Ref< editor::IPipelineCache > pipelineCache;
	if (settings->getProperty< bool >(L"Pipeline.AvalancheCache", false))
	{
		pipelineCache = new editor::AvalanchePipelineCache();
		if (!pipelineCache->create(settings))
		{
			traktor::log::warning << L"Unable to create pipeline avalanche cache; cache disabled." << Endl;
			pipelineCache = nullptr;
		}
		else
			log::info << L"Avalanche pipeline cache created successfully." << Endl;
	}
	else if (settings->getProperty< bool >(L"Pipeline.FileCache", false))
	{
		pipelineCache = new editor::FilePipelineCache();
		if (!pipelineCache->create(settings))
		{
			traktor::log::warning << L"Unable to create pipeline file cache; cache disabled." << Endl;
			pipelineCache = nullptr;
		}
		else
			log::info << L"File pipeline cache created successfully." << Endl;
	}
	else
		log::info << L"Pipeline cache disabled." << Endl;

	// Create pipeline factory.
	editor::PipelineFactory pipelineFactory(settings);
	editor::PipelineDependencySet pipelineDependencySet;

	// Collect dependencies.
	Ref< editor::IPipelineDepends > pipelineDepends;
	if (settings->getProperty< bool >(L"Pipeline.DependsThreads", true))
	{
		pipelineDepends = new editor::PipelineDependsParallel(
			&pipelineFactory,
			sourceDatabaseAndCache.database,
			outputDatabaseAndCache.database,
			&pipelineDependencySet,
			pipelineDb,
			sourceDatabaseAndCache.cache
		);
	}
	else
	{
		pipelineDepends = new editor::PipelineDependsIncremental(
			&pipelineFactory,
			sourceDatabaseAndCache.database,
			outputDatabaseAndCache.database,
			&pipelineDependencySet,
			pipelineDb,
			sourceDatabaseAndCache.cache
		);
	}

	traktor::log::info << L"Collecting dependencies..." << Endl;
	traktor::log::info << IncreaseIndent;

	pipelineDb->beginTransaction();

	const std::vector< Guid >& roots = params.getRoots();
	if (!roots.empty())
	{
		for (const auto& root : roots)
		{
			traktor::log::info << L"Traversing root \"" << root.format() << L"\"..." << Endl;
			pipelineDepends->addDependency(root, editor::PdfBuild);
		}
	}
	else
	{
		RefArray< db::Instance > assetInstances;
		db::recursiveFindChildInstances(sourceDatabaseAndCache.database->getRootGroup(), db::FindInstanceByType(type_of< editor::Assets >()), assetInstances);
		for (auto assetInstance : assetInstances)
		{
			traktor::log::info << L"Traversing root \"" << assetInstance->getGuid().format() << L"\"..." << Endl;
			pipelineDepends->addDependency(assetInstance, editor::PdfBuild);
		}
	}

	pipelineDepends->waitUntilFinished();

	traktor::log::info << DecreaseIndent;

	// Write dependency set for debugging.
#if 0
	Ref< traktor::IStream > f = FileSystem::getInstance().open(L"Dependencies.txt", File::FmWrite);
	pipelineDependencySet.dump(FileOutputStream(f, new Utf8Encoding()));
#endif

	AutoPtr< StatusListener > statusListener;
	if (params.getProgress())
		statusListener.reset(new StatusListener());

	// Build output.
	editor::PipelineBuilder pipelineBuilder(
		&pipelineFactory,
		sourceDatabaseAndCache.database,
		outputDatabaseAndCache.database,
		pipelineCache,
		pipelineDb,
		sourceDatabaseAndCache.cache,
		statusListener.ptr(),
		params.getVerbose()
	);

	if (params.getRebuild())
		traktor::log::info << L"Rebuilding " << pipelineDependencySet.size() << L" asset(s)..." << Endl;
	else
		traktor::log::info << L"Building " << pipelineDependencySet.size() << L" asset(s)..." << Endl;

	traktor::log::info << IncreaseIndent;

	Thread* bt = ThreadManager::getInstance().create([&](){ threadBuild(pipelineBuilder, &pipelineDependencySet, params.getRebuild()); }, L"Build thread");

	// Execute build thread; keep watching if we've
	// received a break signal thus terminate thread early.
	bt->start();
	while (!bt->wait(100))
	{
		if (g_receivedBreakSignal)
		{
			traktor::log::info << L"Received BREAK signal; aborting build..." << Endl;
			bt->stop();
			bt->wait();
		}
	}

	ThreadManager::getInstance().destroy(bt);

	traktor::log::info << DecreaseIndent;
	traktor::log::info << L"Finished" << Endl;

	pipelineDb->endTransaction();
	pipelineDb->close();

	return g_success;
}

int standalone(const CommandLine& cmdLine)
{
#if defined(_WIN32)
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)consoleCtrlHandler, TRUE);
#endif

	Ref< traktor::IStream > logFile;

	if (cmdLine.hasOption('l', L"log"))
	{
		RefArray< File > logs = FileSystem::getInstance().find(L"Pipeline_*.log");

		// Get "alive" log ids.
		std::vector< int32_t > logIds;
		for (auto log : logs)
		{
			std::wstring logName = log->getPath().getFileNameNoExtension();
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
				ss << L"Pipeline_" << logIds.front() << L".log";
				FileSystem::getInstance().remove(ss.str());
				logIds.erase(logIds.begin());
			}

			nextLogId = logIds.back() + 1;
		}

		// Create new log file.
		StringOutputStream ss;
		ss << L"Pipeline_" << nextLogId << L".log";
		if ((logFile = FileSystem::getInstance().open(ss.str(), File::FmWrite)) != nullptr)
		{
			Ref< FileOutputStream > logStream = new FileOutputStream(logFile, new Utf8Encoding());
			Ref< LogStreamTarget > logStreamTarget = new LogStreamTarget(logStream);

			traktor::log::info   .setGlobalTarget(new LogRedirectTarget(logStreamTarget, traktor::log::info   .getGlobalTarget()));
			traktor::log::warning.setGlobalTarget(new LogRedirectTarget(logStreamTarget, traktor::log::warning.getGlobalTarget()));
			traktor::log::error  .setGlobalTarget(new LogRedirectTarget(logStreamTarget, traktor::log::error  .getGlobalTarget()));

			traktor::log::info << L"Log file \"" << ss.str() << L"\" created." << Endl;
		}
		else
			traktor::log::error << L"Unable to create log file; logging only to std pipes." << Endl;
	}

	std::vector< Guid > roots;
	if (cmdLine.getCount() > 0)
	{
		for (uint32_t i = 0; i < cmdLine.getCount(); ++i)
		{
			Guid assetGuid(cmdLine.getString(i));
			if (assetGuid.isNull() || !assetGuid.isValid())
			{
				traktor::log::error << L"Invalid root asset guid (" << i << L")." << Endl;
				return 1;
			}
			roots.push_back(assetGuid);
		}
	}

	std::wstring settingsFile = L"Traktor.Editor";
	if (cmdLine.hasOption('s', L"settings"))
		settingsFile = cmdLine.getOption('s', L"settings").getString();

	PipelineParameters params(
		OS::getInstance().getEnvironment(),
		FileSystem::getInstance().getAbsolutePath(L"").getPathName(),
		settingsFile,
		cmdLine.hasOption('v', L"verbose"),
		cmdLine.hasOption('p', L"progress"),
		cmdLine.hasOption('f', L"force"),
		cmdLine.hasOption('n', L"no-cache"),
		roots
	);

	bool success = perform(params);

	traktor::log::info << L"Bye" << Endl;
	return success ? 0 : 1;
}

int main(int argc, const char** argv)
{
	int32_t result = 1;

	traktor::log::info << L"Pipeline; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'." << Endl;

#if defined(__LINUX__) || defined(__RPI__)
	signal(SIGSEGV, signalExceptionHandler);
#endif

#if !defined(_DEBUG)
	try
#endif
	{
#if defined(_WIN32) && !defined(_DEBUG)
		SetErrorMode(SEM_NOGPFAULTERRORBOX);
		PVOID eh = AddVectoredExceptionHandler(1, exceptionVectoredHandler);
#endif

		CommandLine cmdLine(argc, argv);
		result = standalone(cmdLine);

#if defined(_WIN32) && !defined(_DEBUG)
		RemoveVectoredExceptionHandler(eh);
#endif
	}
#if !defined(_DEBUG)
	catch (...)
	{
#	if defined(_WIN32)
		HMODULE hCrashModule;
		if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCTSTR>(g_exceptionAddress), &hCrashModule))
		{
			TCHAR fileName[MAX_PATH];
			GetModuleFileName(hCrashModule, fileName, sizeof_array(fileName));
			log::error << L"Unhandled exception occurred at 0x" << (uint64_t)g_exceptionAddress << L" in module " << (uint64_t)hCrashModule << L" " << fileName << L"." << Endl;
		}
		else
			log::error << L"Unhandled exception occurred at 0x" << (uint64_t)g_exceptionAddress << L"." << Endl;
#	else
		log::error << L"Unhandled exception occurred." << Endl;
#	endif
	}
#endif

	return result;
}
