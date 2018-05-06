/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/System/IProcess.h"
#include "Core/System/ISharedMemory.h"
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
#include "Editor/Pipeline/AgentBuild.h"
#include "Editor/Pipeline/AgentConnect.h"
#include "Editor/Pipeline/AgentStatus.h"
#include "Editor/Pipeline/FilePipelineCache.h"
#include "Editor/Pipeline/MemCachedPipelineCache.h"
#include "Editor/Pipeline/PipelineBuilder.h"
#include "Editor/Pipeline/PipelineDbFlat.h"
#include "Editor/Pipeline/PipelineDependencySet.h"
#include "Editor/Pipeline/PipelineDependsIncremental.h"
#include "Editor/Pipeline/PipelineDependsParallel.h"
#include "Editor/Pipeline/PipelineFactory.h"
#include "Editor/Pipeline/PipelineInstanceCache.h"
#include "Editor/Pipeline/PipelineSettings.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/Network.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/NetworkService.h"
#include "Pipeline/App/PipelineBuilderWrapper.h"
#include "Pipeline/App/PipelineLog.h"
#include "Pipeline/App/PipelineParameters.h"
#include "Pipeline/App/PipelineResult.h"
#include "Pipeline/App/ReadOnlyObjectCache.h"
#include "Xml/XmlDeserializer.h"

#if defined(_WIN32)
#	include <windows.h>
#	include <tlhelp32.h>
#	include "Pipeline/App/Win32/StackWalker.h"
#endif

using namespace traktor;

const uint16_t c_defaultIPCPort = 52412;

#if defined(_WIN32)

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

class LogStreamTarget : public ILogTarget
{
public:
	LogStreamTarget(OutputStream* stream)
	:	m_stream(stream)
	{
	}

	virtual void log(uint32_t threadId, int32_t level, const std::wstring& str) T_OVERRIDE T_FINAL
	{
		(*m_stream) << str << Endl;
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

class LogRedirect : public ILogTarget
{
public:
	LogRedirect(
		ILogTarget* originalTarget,
		net::BidirectionalObjectTransport* transport
	)
	:	m_originalTarget(originalTarget)
	,	m_transport(transport)
	{
	}

	virtual void log(uint32_t threadId, int32_t level, const std::wstring& str) T_OVERRIDE T_FINAL
	{
		if (m_originalTarget)
			m_originalTarget->log(threadId, level, str);

		if (m_transport->connected())
		{
			const PipelineLog tlog(threadId, level, str);
			m_transport->send(&tlog);
		}
	}

private:
	Ref< ILogTarget > m_originalTarget;
	Ref< net::BidirectionalObjectTransport > m_transport;
};

struct StatusListener : public editor::IPipelineBuilder::IListener
{
	virtual void beginBuild(
		int32_t core,
		int32_t index,
		int32_t count,
		const editor::PipelineDependency* dependency
	) const T_OVERRIDE T_FINAL
	{
		log::info << L":" << index << L":" << count << Endl;
	}

	virtual void endBuild(
		int32_t core,
		int32_t index,
		int32_t count,
		const editor::PipelineDependency* dependency,
		editor::IPipelineBuilder::BuildResult result
	) const T_OVERRIDE T_FINAL
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
		settings = xml::XmlDeserializer(file).readObject< PropertyGroup >();
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
		globalSettings = xml::XmlDeserializer(file).readObject< PropertyGroup >();
		if (!globalSettings)
			log::warning << globalConfig << L" corrupt!" << Endl;
		file->close();
	}

    // Read system properties.
    if ((file = FileSystem::getInstance().open(systemConfig, File::FmRead)) != 0)
    {
        systemSettings = xml::XmlDeserializer(file).readObject< PropertyGroup >();
		if (!systemSettings)
			log::warning << systemConfig << L" corrupt!" << Endl;
        file->close();
	}

	// Merge in system configuration.
	if (globalSettings && systemSettings)
		settings = globalSettings->mergeReplace(systemSettings);
	else
		settings = globalSettings;

	return settings;
}

struct ConnectionAndCache 
{
	Ref< db::Database > database;
	Ref< editor::PipelineInstanceCache > cache;
};

Mutex g_pipelineMutex(Guid(L"{91B42B2E-652D-4251-BA5B-9683F30518DD}"));
bool g_receivedBreakSignal = false;
bool g_success = false;
std::map< std::wstring, ConnectionAndCache > g_databaseConnections;
std::set< std::wstring > g_loadedModules;

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
	std::map< std::wstring, ConnectionAndCache >::iterator i = g_databaseConnections.find(connectionString);
	if (i != g_databaseConnections.end())
		return i->second;

	Ref< db::Database > database = new db::Database();
	if (!database->open(connectionString))
	{
		if (!create || !database->create(connectionString))
			return ConnectionAndCache();
	}

	g_databaseConnections[connectionString].database = database;

	if (!create)
	{
		std::wstring cachePath = settings->getProperty< std::wstring >(L"Pipeline.CachePath");
		g_databaseConnections[connectionString].cache = new editor::PipelineInstanceCache(database, cachePath);
	}

	return g_databaseConnections[connectionString];
}

void updateDatabases()
{
	Ref< const db::IEvent > event;
	bool remote;

	for (std::map< std::wstring, ConnectionAndCache >::iterator i = g_databaseConnections.begin(); i != g_databaseConnections.end(); ++i)
	{
		while (i->second.database->getEvent(event, remote))
		{
			if (remote)
			{
				if (const db::EvtInstanceCreated* instanceCreated = dynamic_type_cast< const db::EvtInstanceCreated* >(event))
				{
					Ref< db::Instance > instance = i->second.database->getInstance(instanceCreated->getInstanceGuid());
					if (instance)
						log::info << L"Database event; instance \"" << instance->getName() << L"\" created" << Endl;
					else
						log::info << L"Database event; instance \"" << instanceCreated->getInstanceGuid().format() << L"\" created" << Endl;

					if (i->second.cache)
						i->second.cache->flush(instanceCreated->getInstanceGuid());
				}
				else if (const db::EvtInstanceCommitted* instanceCommited = dynamic_type_cast< const db::EvtInstanceCommitted* >(event))
				{
					Ref< db::Instance > instance = i->second.database->getInstance(instanceCommited->getInstanceGuid());
					if (instance)
						log::info << L"Database event; instance \"" << instance->getName() << L"\" committed" << Endl;
					else
						log::info << L"Database event; instance \"" << instanceCommited->getInstanceGuid().format() << L"\" committed" << Endl;

					if (i->second.cache)
						i->second.cache->flush(instanceCommited->getInstanceGuid());
				}
				else if (const db::EvtInstanceRemoved* instanceRemoved = dynamic_type_cast< const db::EvtInstanceRemoved* >(event))
				{
					log::info << L"Database event; instance \"" << instanceRemoved->getInstanceGuid().format() << L"\" removed" << Endl;
					if (i->second.cache)
						i->second.cache->flush(instanceRemoved->getInstanceGuid());
				}
				else
					log::info << L"Database event; " << type_name(event) << Endl;
			}
		}
	}
}

bool perform(const PipelineParameters* params)
{
	if (!FileSystem::getInstance().setCurrentVolumeAndDirectory(params->getWorkingDirectory()))
	{
		traktor::log::error << L"Unable to change working directory" << Endl;
		return false;
	}

	if (params->getVerbose())
		traktor::log::info << L"Using settings \"" << params->getSettings() << L"\"." << Endl;

	Ref< PropertyGroup > settings = loadSettings(params->getSettings());
	if (!settings)
	{
		traktor::log::error << L"Unable to load pipeline settings \"" << params->getSettings() << L"\"" << Endl;
		return false;
	}

	if (params->getNoCache())
	{
		settings->setProperty< PropertyBoolean >(L"Pipeline.MemCached", false);
		settings->setProperty< PropertyBoolean >(L"Pipeline.FileCache", false);
	}

	std::set< std::wstring > modulePaths = settings->getProperty< std::set< std::wstring > >(L"Editor.ModulePaths");
	std::set< std::wstring > modules = settings->getProperty< std::set< std::wstring > >(L"Editor.Modules");

	std::vector< Path > modulePathsFlatten(modulePaths.begin(), modulePaths.end());
	for (std::set< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
	{
		if (g_loadedModules.find(*i) == g_loadedModules.end())
		{
			Library library;
			if (!library.open(*i, modulePathsFlatten, true))
			{
				traktor::log::error << L"Unable to load module \"" << *i << L"\"" << Endl;
				return false;
			}

			if (params->getVerbose())
				traktor::log::info << L"Library \"" << library.getPath().getPathName() << L"\" loaded." << Endl;

			library.detach();
			g_loadedModules.insert(*i);
		}
	}

	std::wstring sourceDatabaseCS = settings->getProperty< std::wstring >(L"Editor.SourceDatabase");
	std::wstring outputDatabaseCS = settings->getProperty< std::wstring >(L"Editor.OutputDatabase");
	
	ConnectionAndCache sourceDatabaseAndCache = openDatabase(settings, sourceDatabaseCS, false);
	if (!sourceDatabaseAndCache.database)
	{
		traktor::log::error << L"Unable to open source database \"" << sourceDatabaseCS << L"\"" << Endl;
		return false;
	}

	ConnectionAndCache outputDatabaseAndCache = openDatabase(settings, outputDatabaseCS, true);
	if (!outputDatabaseAndCache.database)
	{
		traktor::log::error << L"Unable to open or create output database \"" << outputDatabaseCS << L"\"" << Endl;
		return false;
	}

	std::wstring connectionString = settings->getProperty< std::wstring >(L"Pipeline.Db");
	std::wstring cachePath = settings->getProperty< std::wstring >(L"Pipeline.CachePath");

	Ref< editor::IPipelineDb > pipelineDb = new editor::PipelineDbFlat();
	if (!pipelineDb->open(connectionString))
	{
		traktor::log::error << L"Unable to connect to pipeline database" << Endl;
		return false;
	}

	// Create cache if enabled.
	Ref< editor::IPipelineCache > pipelineCache;
	if (settings->getProperty< bool >(L"Pipeline.MemCached", false))
	{
		pipelineCache = new editor::MemCachedPipelineCache();
		if (!pipelineCache->create(settings))
		{
			traktor::log::warning << L"Unable to create pipeline cache; cache disabled" << Endl;
			pipelineCache = 0;
		}
	}
	if (settings->getProperty< bool >(L"Pipeline.FileCache", false))
	{
		pipelineCache = new editor::FilePipelineCache();
		if (!pipelineCache->create(settings))
		{
			traktor::log::warning << L"Unable to create pipeline file cache; cache disabled" << Endl;
			pipelineCache = 0;
		}
	}

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

	const std::vector< Guid >& roots = params->getRoots();
	if (!roots.empty())
	{
		for (std::vector< Guid >::const_iterator i = roots.begin(); i != roots.end(); ++i)
		{
			traktor::log::info << L"Traversing root \"" << i->format() << L"\"..." << Endl;
			pipelineDepends->addDependency(*i, editor::PdfBuild);
		}
	}
	else
	{
		RefArray< db::Instance > assetInstances;
		db::recursiveFindChildInstances(sourceDatabaseAndCache.database->getRootGroup(), db::FindInstanceByType(type_of< editor::Assets >()), assetInstances);
		for (RefArray< db::Instance >::iterator i = assetInstances.begin(); i != assetInstances.end(); ++i)
		{
			traktor::log::info << L"Traversing root \"" << (*i)->getGuid().format() << L"\"..." << Endl;
			pipelineDepends->addDependency(*i, editor::PdfBuild);
		}
	}

	pipelineDepends->waitUntilFinished();

	traktor::log::info << DecreaseIndent;

	AutoPtr< StatusListener > statusListener;
	if (params->getProgress())
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
		settings->getProperty< bool >(L"Pipeline.BuildThreads", true)
	);

	if (params->getRebuild())
		traktor::log::info << L"Rebuilding " << pipelineDependencySet.size() << L" asset(s)..." << Endl;
	else
		traktor::log::info << L"Building " << pipelineDependencySet.size() << L" asset(s)..." << Endl;

	traktor::log::info << IncreaseIndent;

	Thread* bt = ThreadManager::getInstance().create(
		makeStaticFunctor< editor::PipelineBuilder&, const editor::PipelineDependencySet*, bool >(&threadBuild, pipelineBuilder, &pipelineDependencySet, params->getRebuild()),
		L"Build thread"
	);

	// Execute build thread; keep watching if we've
	// received a break signal thus terminate thread early.
	bt->start(Thread::Above);
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

int slave(const CommandLine& cmdLine)
{
#if defined(_WIN32)
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)consoleCtrlHandler, TRUE);
#endif

	uint16_t port = c_defaultIPCPort;
	if (cmdLine.hasOption(L"port"))
		port = uint16_t(cmdLine.getOption(L"port").getInteger());

	net::TcpSocket socket;
	if (!socket.bind(net::SocketAddressIPv4(port)))
	{
		traktor::log::error << L"Unable to bind socket to port " << port << Endl;
		return 1;
	}

	traktor::log::info << L"Waiting..." << Endl;

	if (!socket.listen())
	{
		traktor::log::error << L"Unable to listen on socket" << Endl;
		return 1;
	}

#if defined(_WIN32)

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	DWORD dwParentPID = 0;
	
	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnapshot, &pe))
	{
		DWORD dwPID = GetCurrentProcessId();
		do 
		{
			if (pe.th32ProcessID == dwPID)
			{
				dwParentPID = pe.th32ParentProcessID;
				break;
			}
		}
		while (Process32Next(hSnapshot, &pe));
	}

	CloseHandle(hSnapshot);

	HANDLE hParentProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, FALSE, dwParentPID);

#endif

	while (!g_receivedBreakSignal)
	{
#if defined(_WIN32)
		if (hParentProcess != NULL && WaitForSingleObject(hParentProcess, 0) == WAIT_OBJECT_0)
			break;
#endif

		updateDatabases();

		if (socket.select(true, false, false, 250) <= 0)
			continue;

		Ref< net::TcpSocket > client = socket.accept();
		if (!client)
			continue;

		client->setNoDelay(true);

		Ref< net::BidirectionalObjectTransport > transport = new net::BidirectionalObjectTransport(client);

		Ref< ILogTarget > infoTarget    = traktor::log::info.   getGlobalTarget();
		Ref< ILogTarget > warningTarget = traktor::log::warning.getGlobalTarget();
		Ref< ILogTarget > errorTarget   = traktor::log::error.  getGlobalTarget();

		traktor::log::info   .setGlobalTarget(new LogRedirect(infoTarget,    transport));
		traktor::log::warning.setGlobalTarget(new LogRedirect(warningTarget, transport));
		traktor::log::error  .setGlobalTarget(new LogRedirect(errorTarget,   transport));

		Ref< PipelineParameters > params;
		transport->recv< PipelineParameters >(1000, params);

		bool success = false;
		if (params)
			success = perform(params);
		else
			traktor::log::error << L"Unable to read pipeline parameters" << Endl;

		const PipelineResult result(success ? 0 : 1);
		transport->send(&result);

		traktor::log::info   .setGlobalTarget(infoTarget);
		traktor::log::warning.setGlobalTarget(warningTarget);
		traktor::log::error  .setGlobalTarget(errorTarget);

		transport->close();
		transport = 0;
	}

#if defined(_WIN32)
	CloseHandle(hParentProcess);
#endif

	traktor::log::info << L"Bye" << Endl;
	return 0;
}

int master(const CommandLine& cmdLine)
{
	Ref< traktor::IStream > logFile;
	int32_t result = 1;

	if (cmdLine.hasOption('l', L"log"))
	{
		std::wstring logPath = cmdLine.getOption('l', L"log").getString();
		if ((logFile = FileSystem::getInstance().open(logPath, File::FmWrite)) != 0)
		{
			Ref< FileOutputStream > logStream = new FileOutputStream(logFile, new Utf8Encoding());
			Ref< LogStreamTarget > logStreamTarget = new LogStreamTarget(logStream);

			traktor::log::info   .setGlobalTarget(new LogDualTarget(logStreamTarget, traktor::log::info   .getGlobalTarget()));
			traktor::log::warning.setGlobalTarget(new LogDualTarget(logStreamTarget, traktor::log::warning.getGlobalTarget()));
			traktor::log::error  .setGlobalTarget(new LogDualTarget(logStreamTarget, traktor::log::error  .getGlobalTarget()));

			traktor::log::info << L"Log file \"Application.log\" created" << Endl;
		}
		else
			traktor::log::error << L"Unable to create log file; logging only to std pipes" << Endl;
	}

	uint16_t port = c_defaultIPCPort;
	if (cmdLine.hasOption(L"port"))
		port = uint16_t(cmdLine.getOption(L"port").getInteger());

	if (!g_pipelineMutex.existing())
	{
		Path executable = OS::getInstance().getExecutable();

		Ref< IProcess > slaveProcess = OS::getInstance().execute(
			executable.getPathName() + L" -slave",
			L"",
			OS::getInstance().getEnvironment(),
			false,
			false,
			true
		);
		if (!slaveProcess)
			return 1;

		slaveProcess->setPriority(IProcess::Below);
	}

	std::vector< Guid > roots;
	if (cmdLine.getCount() > 0)
	{
		for (int32_t i = 0; i < cmdLine.getCount(); ++i)
		{
			Guid assetGuid(cmdLine.getString(i));
			if (assetGuid.isNull() || !assetGuid.isValid())
			{
				traktor::log::error << L"Invalid root asset guid (" << i << L" \"" << cmdLine.getString(i) << L"\")" << Endl;
				return 1;
			}
			roots.push_back(assetGuid);
		}
	}

	Ref< net::TcpSocket > socket = new net::TcpSocket();
	if (!socket->connect(net::SocketAddressIPv4(L"localhost", port)))
	{
		traktor::log::error << L"Unable to establish connection with pipeline slave using port " << port << Endl;
		return 1;
	}

	socket->setNoDelay(true);

	std::wstring settingsFile = L"Traktor.Editor";
	if (cmdLine.hasOption('s', L"settings"))
		settingsFile = cmdLine.getOption('s', L"settings").getString();

	PipelineParameters params(
		FileSystem::getInstance().getAbsolutePath(L"").getPathName(),
		settingsFile,
		cmdLine.hasOption('v', L"verbose"),
		cmdLine.hasOption('p', L"progress"),
		cmdLine.hasOption('f', L"force"),
		cmdLine.hasOption('n', L"no-cache"),
		roots
	);

	Ref< net::BidirectionalObjectTransport > transport = new net::BidirectionalObjectTransport(socket);

	transport->send(&params);
	for (;;)
	{
		Ref< ISerializable > slaveMessage;
		if (transport->recv< ISerializable >(1000, slaveMessage) == net::BidirectionalObjectTransport::RtDisconnected)
			break;

		if (const PipelineLog* slaveLog = dynamic_type_cast< const PipelineLog* >(slaveMessage))
		{
			switch (slaveLog->getLevel())
			{
			default:
			case 0:
				traktor::log::info << slaveLog->getText() << Endl;
				break;
			case 1:
				traktor::log::warning << slaveLog->getText() << Endl;
				break;
			case 2:
				traktor::log::error << slaveLog->getText() << Endl;
				break;
			case 3:
				traktor::log::debug << slaveLog->getText() << Endl;
				break;
			}
		}
		else if (const PipelineResult* slaveResult = dynamic_type_cast< const PipelineResult* >(slaveMessage))
		{
			result = slaveResult->getResult();
		}
	}

	transport->close();
	transport = 0;

	if (logFile)
	{
		traktor::log::info.setBuffer(0);
		traktor::log::warning.setBuffer(0);
		traktor::log::error.setBuffer(0);
		traktor::log::debug.setBuffer(0);

		logFile->close();
		logFile = 0;
	}

	return result;
}

int standalone(const CommandLine& cmdLine)
{
#if defined(_WIN32)
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)consoleCtrlHandler, TRUE);
#endif

	traktor::log::info << L"Standalone Mode" << Endl;

	Ref< traktor::IStream > logFile;

	if (cmdLine.hasOption('l', L"log"))
	{
		std::wstring logPath = cmdLine.getOption('l', L"log").getString();
		if ((logFile = FileSystem::getInstance().open(logPath, File::FmWrite)) != 0)
		{
			Ref< FileOutputStream > logStream = new FileOutputStream(logFile, new Utf8Encoding());
			Ref< LogStreamTarget > logStreamTarget = new LogStreamTarget(logStream);

			traktor::log::info   .setGlobalTarget(new LogDualTarget(logStreamTarget, traktor::log::info   .getGlobalTarget()));
			traktor::log::warning.setGlobalTarget(new LogDualTarget(logStreamTarget, traktor::log::warning.getGlobalTarget()));
			traktor::log::error  .setGlobalTarget(new LogDualTarget(logStreamTarget, traktor::log::error  .getGlobalTarget()));

			traktor::log::info << L"Log file \"Application.log\" created" << Endl;
		}
		else
			traktor::log::error << L"Unable to create log file; logging only to std pipes" << Endl;
	}

	std::vector< Guid > roots;
	if (cmdLine.getCount() > 0)
	{
		for (int32_t i = 0; i < cmdLine.getCount(); ++i)
		{
			Guid assetGuid(cmdLine.getString(i));
			if (assetGuid.isNull() || !assetGuid.isValid())
			{
				traktor::log::error << L"Invalid root asset guid (" << i << L")" << Endl;
				return 1;
			}
			roots.push_back(assetGuid);
		}
	}

	std::wstring settingsFile = L"Traktor.Editor";
	if (cmdLine.hasOption('s', L"settings"))
		settingsFile = cmdLine.getOption('s', L"settings").getString();

	PipelineParameters params(
		FileSystem::getInstance().getAbsolutePath(L"").getPathName(),
		settingsFile,
		cmdLine.hasOption('v', L"verbose"),
		cmdLine.hasOption('p', L"progress"),
		cmdLine.hasOption('f', L"force"),
		cmdLine.hasOption('n', L"no-cache"),
		roots
	);

	bool success = perform(&params);

	traktor::log::info << L"Bye" << Endl;
	return success ? 0 : 1;
}

void threadProcessAgentClient(
	Ref< editor::AgentConnect > agentConnect,
	Ref< net::BidirectionalObjectTransport > transport,
	Ref< db::Database > sourceDatabase,
	Ref< db::Database > outputDatabase,
	Ref< ReadOnlyObjectCache > objectCache
)
{
	// Create pipeline environment.
	Ref< editor::PipelineSettings > pipelineSettings = new editor::PipelineSettings(agentConnect->getSettings());
	Ref< editor::PipelineFactory > pipelineFactory = new editor::PipelineFactory(agentConnect->getSettings());
	Ref< PipelineBuilderWrapper > pipelineBuilder = new PipelineBuilderWrapper(
		pipelineFactory,
		transport,
		agentConnect->getHost(),
		agentConnect->getStreamServerPort(),
		sourceDatabase,
		outputDatabase,
		objectCache
	);

	log::info << L"Ready; waiting for build items..." << Endl;

	// Agent connected and ready; wait for build items.
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	while (!currentThread->stopped() && transport->connected())
	{
		Ref< editor::AgentBuild > agentBuild;
		if (transport->recv< editor::AgentBuild >(100, agentBuild) != 1)
			continue;

		log::info << L"Received build item " << agentBuild->getOutputGuid().format() << Endl;

		const TypeInfo* pipelineType = TypeInfo::find(agentBuild->getPipelineTypeName().c_str());
		if (!pipelineType)
		{
			log::error << L"Agent build error; no such pipeline \"" << agentBuild->getPipelineTypeName() << L"\"" << Endl;
			continue;
		}
	
		Ref< editor::IPipeline > pipeline = pipelineFactory->findPipeline(*pipelineType);
		if (!pipeline)
		{
			log::error << L"Agent build error; unable to get pipeline" << Endl;
			continue;
		}

		log::info << L"Executing build using pipeline \"" << type_name(pipeline) << L"\"..." << Endl;

		bool result = pipeline->buildOutput(
			pipelineBuilder,
			0,
			0,
			agentBuild->getSourceInstanceGuid().isNotNull() ? sourceDatabase->getInstance(agentBuild->getSourceInstanceGuid()) : 0,
			agentBuild->getSourceAsset(),
			agentBuild->getSourceAssetHash(),
			agentBuild->getOutputPath(),
			agentBuild->getOutputGuid(),
			0,
			agentBuild->getReason()
		);

		editor::AgentStatus agentStatus(
			agentBuild->getOutputGuid(),
			result
		);
		transport->send(&agentStatus);

		if (result)
			log::info << L"Build succeeded" << Endl;
		else
			log::info << L"Build failed" << Endl;
	}

	transport->close();
	transport = 0;
}

int agent(const CommandLine& cmdLine)
{
	std::wstring settingsFile = L"Traktor.Editor";
	if (cmdLine.hasOption('s', L"settings"))
		settingsFile = cmdLine.getOption('s', L"settings").getString();

	Ref< PropertyGroup > settings = loadSettings(settingsFile);
	if (!settings)
	{
		traktor::log::error << L"Unable to load settings \"" << settingsFile << L"\"" << Endl;
		return 1;
	}

	int32_t agentCount = OS::getInstance().getCPUCoreCount();
	if (cmdLine.hasOption('n', L"agents"))
	{
		agentCount = cmdLine.getOption('n', L"agents").getInteger();
		if (agentCount <= 0)
		{
			traktor::log::error << L"Invalid number of agents; must be atleast one" << Endl;
			return 1;
		}
	}

	std::set< std::wstring > modules = settings->getProperty< std::set< std::wstring > >(L"Editor.Modules");
	for (std::set< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
	{
		if (g_loadedModules.find(*i) == g_loadedModules.end())
		{
			Library library;
			if (!library.open(*i))
			{
				traktor::log::warning << L"Unable to load module \"" << *i << L"\"" << Endl;
				continue;
			}

			if (cmdLine.hasOption('v', L"verbose"))
				traktor::log::info << L"Library \"" << library.getPath().getPathName() << L"\" loaded." << Endl;

			library.detach();
			g_loadedModules.insert(*i);
		}
	}

	net::Network::initialize();

	Ref< net::TcpSocket > serverSocket = new net::TcpSocket();
	if (!serverSocket->bind(net::SocketAddressIPv4()))
	{
		traktor::log::error << L"Unable to bind server socket to port" << Endl;
		return 1;
	}

	if (!serverSocket->listen())
	{
		traktor::log::error << L"Unable to listen on server socket" << Endl;
		return 2;
	}

	uint16_t listenPort = dynamic_type_cast< net::SocketAddressIPv4* >(serverSocket->getLocalAddress())->getPort();

	Ref< net::DiscoveryManager > discoveryManager = new net::DiscoveryManager();
	if (!discoveryManager->create(net::MdPublishServices))
	{
		traktor::log::error << L"Unable to create discovery manager" << Endl;
		return 3;
	}

	net::SocketAddressIPv4::Interface itf;
	if (!net::SocketAddressIPv4::getBestInterface(itf))
	{
		traktor::log::error << L"Unable to get interfaces" << Endl;
		return 4;
	}

	Ref< PropertyGroup > properties = new PropertyGroup();
	properties->setProperty< PropertyString >(L"Description", OS::getInstance().getComputerName());
	properties->setProperty< PropertyString >(L"Host", itf.addr->getHostName());
	properties->setProperty< PropertyInteger >(L"Port", listenPort);
	properties->setProperty< PropertyInteger >(L"Agents", agentCount);

	std::set< std::wstring > types;

	/*
	TypeInfoSet pipelineTypes;
	type_of< editor::IPipeline >().findAllOf(pipelineTypes, false);
	for (TypeInfoSet::const_iterator i = pipelineTypes.begin(); i != pipelineTypes.end(); ++i)
		types.insert((*i)->getName());

	properties->setProperty< PropertyStringSet >(L"PipelineTypes", types);
	*/

	discoveryManager->addService(new net::NetworkService(L"Pipeline/Agent2", properties));

	traktor::log::info << L"Discoverable as \"Pipeline/Agent2\", host \"" << itf.addr->getHostName() << L"\" on port " << listenPort << Endl;
	traktor::log::info << L"Waiting for client(s)..." << Endl;

	std::map< std::wstring, Ref< db::Database > > databases;
	std::map< std::wstring, Ref< ReadOnlyObjectCache > > objectCaches;
	std::list< Thread* > clientThreads;
	for (;;)
	{
		if (serverSocket->select(true, false, false, 1000) <= 0)
		{
			for (std::list< Thread* >::iterator i = clientThreads.begin(); i != clientThreads.end(); )
			{
				if ((*i)->wait(0))
				{
					traktor::log::info << L"Client thread destroyed" << Endl;
					ThreadManager::getInstance().destroy(*i);
					i = clientThreads.erase(i);
				}
				else
					++i;
			}
			continue;
		}

		Ref< net::TcpSocket > clientSocket = serverSocket->accept();
		if (!clientSocket)
			continue;

		Ref< net::BidirectionalObjectTransport > clientTransport = new net::BidirectionalObjectTransport(clientSocket);

		Ref< editor::AgentConnect > agentConnect;
		if (clientTransport->recv< editor::AgentConnect >(10000, agentConnect) != 1)
		{
			log::error << L"Agent build error; no AgentConnect message received" << Endl;
			continue;
		}

		log::info << L"Agent connect message received" << Endl;
		log::info << L"\tHost \"" << agentConnect->getHost() << L"\"" << Endl;
		log::info << L"\tDatabase port " << agentConnect->getDatabasePort() << Endl;
		log::info << L"\tStream server port " << agentConnect->getStreamServerPort() << Endl;

		// Setup a remote database connection to source and output databases.
		if (!databases[agentConnect->getSessionId().format() + L"|Source"])
		{
			db::ConnectionString cs;

			cs.set(L"database", agentConnect->getSessionId().format() + L"|Source");
			cs.set(L"host", agentConnect->getHost() + L":" + toString(agentConnect->getDatabasePort()));
			cs.set(L"provider", L"traktor.db.RemoteDatabase");

			log::info << L"Connecting to database \"" << cs.format() << L"\"..." << Endl;

			Ref< db::Database > sourceDatabase = new db::Database();
			if (!sourceDatabase->open(cs))
			{
				log::error << L"Agent build error; unable to open source database" << Endl;
				continue;
			}

			databases[agentConnect->getSessionId().format() + L"|Source"] = sourceDatabase;
			objectCaches[agentConnect->getSessionId().format() + L"|Source"] = new ReadOnlyObjectCache(sourceDatabase);
		}

		if (!databases[agentConnect->getSessionId().format() + L"|Output"])
		{
			db::ConnectionString cs;

			cs.set(L"database", agentConnect->getSessionId().format() + L"|Output");
			cs.set(L"host", agentConnect->getHost() + L":" + toString(agentConnect->getDatabasePort()));
			cs.set(L"provider", L"traktor.db.RemoteDatabase");

			log::info << L"Connecting to database \"" << cs.format() << L"\"..." << Endl;

			Ref< db::Database > outputDatabase = new db::Database();
			if (!outputDatabase->open(cs))
			{
				log::error << L"Agent build error; unable to open output database" << Endl;
				continue;
			}

			databases[agentConnect->getSessionId().format() + L"|Output"] = outputDatabase;
		}

		traktor::log::info << L"Connection established; spawning thread..." << Endl;

		Thread* clientThread = ThreadManager::getInstance().create(
			makeStaticFunctor
			<
				Ref< editor::AgentConnect >,
				Ref< net::BidirectionalObjectTransport >,
				Ref< db::Database >,
				Ref< db::Database >,
				Ref< ReadOnlyObjectCache >
			>
			(
				&threadProcessAgentClient,
				agentConnect,
				clientTransport,
				databases[agentConnect->getSessionId().format() + L"|Source"],
				databases[agentConnect->getSessionId().format() + L"|Output"],
				objectCaches[agentConnect->getSessionId().format() + L"|Source"]
			),
			L"Client thread"
		);
		if (!clientThread)
		{
			traktor::log::error << L"Unable to create client thread" << Endl;
			continue;
		}

		clientThread->start();
		clientThreads.push_back(clientThread);
	}

	return 0;
}

int main(int argc, const char** argv)
{
	int32_t result = 1;

	traktor::log::info << L"Pipeline; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;

	net::Network::initialize();

#if !defined(_DEBUG)
	try
#endif
	{
#if defined(_WIN32) && !defined(_DEBUG)
		SetErrorMode(SEM_NOGPFAULTERRORBOX);
		PVOID eh = AddVectoredExceptionHandler(1, exceptionVectoredHandler);
#endif

		CommandLine cmdLine(argc, argv);
#if !defined(__APPLE__)
		if (cmdLine.hasOption(L"slave"))
			result = slave(cmdLine);
		else if (cmdLine.hasOption(L"standalone"))
			result = standalone(cmdLine);
		else if (cmdLine.hasOption(L"agent"))
			result = agent(cmdLine);
		else
			result = master(cmdLine);
#else
		result = standalone(cmdLine);
#endif

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
			log::error << L"Unhandled exception occurred at 0x" << (uint64_t)g_exceptionAddress << L" in module " << (uint64_t)hCrashModule << L" " << fileName << Endl;
		}
		else
			log::error << L"Unhandled exception occurred at 0x" << (uint64_t)g_exceptionAddress << Endl;
#	else
		log::error << L"Unhandled exception occurred" << Endl;
#	endif
	}
#endif

	net::Network::finalize();
	return result;
}
