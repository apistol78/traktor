//#if defined(_WIN32)
//#	include <Windows.h>
//#endif
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
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/System/ISharedMemory.h"
#include "Core/System/OS.h"
#include "Core/Thread/Mutex.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Database/Compact/CompactDatabase.h"
#include "Database/Local/LocalDatabase.h"
#include "Editor/Assets.h"
#include "Editor/IPipeline.h"
#include "Editor/Pipeline/FilePipelineCache.h"
#include "Editor/Pipeline/MemCachedPipelineCache.h"
#include "Editor/Pipeline/PipelineBuilder.h"
#include "Editor/Pipeline/PipelineDb.h"
#include "Editor/Pipeline/PipelineDependencySet.h"
#include "Editor/Pipeline/PipelineDependsIncremental.h"
#include "Editor/Pipeline/PipelineDependsParallel.h"
#include "Editor/Pipeline/PipelineFactory.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/Network.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketStream.h"
#include "Net/TcpSocket.h"
#include "Xml/XmlDeserializer.h"

using namespace traktor;

class LogStreamTarget : public ILogTarget
{
public:
	LogStreamTarget(OutputStream* stream)
	:	m_stream(stream)
	{
	}

	virtual void log(int32_t level, const std::wstring& str)
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

	virtual void log(int32_t level, const std::wstring& str)
	{
		m_target1->log(level, str);
		m_target2->log(level, str);
	}

private:
	Ref< ILogTarget > m_target1;
	Ref< ILogTarget > m_target2;
};

struct StatusListener : public editor::IPipelineBuilder::IListener
{
	virtual void beginBuild(
		int32_t core,
		int32_t index,
		int32_t count,
		const editor::PipelineDependency* dependency
	) const
	{
		log::info << L":" << index << L":" << count << Endl;
	}

	virtual void endBuild(
		int32_t core,
		int32_t index,
		int32_t count,
		const editor::PipelineDependency* dependency,
		editor::IPipelineBuilder::BuildResult result
	) const
	{
	}
};

Ref< PropertyGroup > loadSettings(const std::wstring& settingsFile)
{
	Ref< PropertyGroup > settings;
	Ref< traktor::IStream > file;

	std::wstring globalConfig = settingsFile + L".config";
	std::wstring userConfig = settingsFile + L"." + OS::getInstance().getCurrentUser() + L".config";

	if ((file = FileSystem::getInstance().open(userConfig, File::FmRead)) != 0)
	{
		settings = xml::XmlDeserializer(file).readObject< PropertyGroup >();
		file->close();
	}

	if (settings)
		return settings;

	if ((file = FileSystem::getInstance().open(globalConfig, File::FmRead)) != 0)
	{
		settings = xml::XmlDeserializer(file).readObject< PropertyGroup >();
		file->close();
	}

	return settings;
}



Mutex g_pipelineMutex(Guid(L"{91B42B2E-652D-4251-BA5B-9683F30518DD}"));
bool g_receivedBreakSignal = false;
bool g_success = false;
std::map< std::wstring, Ref< db::Database > > g_databaseConnections;

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

Ref< db::Database > openDatabase(const std::wstring& connectionString, bool create)
{
	std::map< std::wstring, Ref< db::Database > >::iterator i = g_databaseConnections.find(connectionString);
	if (i != g_databaseConnections.end())
		return i->second;

	Ref< db::Database > database = new db::Database();
	if (!database->open(connectionString))
	{
		if (!create || !database->create(connectionString))
			return 0;
	}

	g_databaseConnections[connectionString] = database;
	return database;
}

void updateDatabases()
{
	Ref< const db::IEvent > event;
	bool remote;

	for (std::map< std::wstring, Ref< db::Database > >::iterator i = g_databaseConnections.begin(); i != g_databaseConnections.end(); ++i)
	{
		while (i->second->getEvent(event, remote))
			;
	}
}


class PipelineParameters : public ISerializable
{
	T_RTTI_CLASS;

public:
	PipelineParameters();

	PipelineParameters(
		const std::wstring& workingDirectory,
		const std::wstring& settings,
		bool progress,
		bool rebuild,
		bool noCache,
		const std::vector< Guid >& roots
	);

	virtual void serialize(ISerializer& s);

	const std::wstring& getWorkingDirectory() const { return m_workingDirectory; }

	const std::wstring& getSettings() const { return m_settings; }

	bool getProgress() const { return m_progress; }

	bool getRebuild() const { return m_rebuild; }

	bool getNoCache() const { return m_noCache; }

	const std::vector< Guid >& getRoots() const { return m_roots; }

private:
	std::wstring m_workingDirectory;
	std::wstring m_settings;
	bool m_progress;
	bool m_rebuild;
	bool m_noCache;
	std::vector< Guid > m_roots;
};

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"PipelineParameters", 0, PipelineParameters, ISerializable)

PipelineParameters::PipelineParameters()
:	m_progress(false)
,	m_rebuild(false)
,	m_noCache(false)
{
}

PipelineParameters::PipelineParameters(
	const std::wstring& workingDirectory,
	const std::wstring& settings,
	bool progress,
	bool rebuild,
	bool noCache,
	const std::vector< Guid >& roots
)
:	m_workingDirectory(workingDirectory)
,	m_settings(settings)
,	m_progress(progress)
,	m_rebuild(rebuild)
,	m_noCache(noCache)
,	m_roots(roots)
{
}

void PipelineParameters::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"workingDirectory", m_workingDirectory);
	s >> Member< std::wstring >(L"settings", m_settings);
	s >> Member< bool >(L"progress", m_progress);
	s >> Member< bool >(L"rebuild", m_rebuild);
	s >> Member< bool >(L"noCache", m_noCache);
	s >> MemberStlVector< Guid >(L"roots", m_roots);
}


class PipelineLog : public ISerializable
{
	T_RTTI_CLASS;

public:
	PipelineLog();

	PipelineLog(int32_t level, const std::wstring& text);

	int32_t getLevel() const { return m_level; }

	const std::wstring& getText() const { return m_text; }

	virtual void serialize(ISerializer& s);

private:
	int32_t m_level;
	std::wstring m_text;
};


T_IMPLEMENT_RTTI_FACTORY_CLASS(L"PipelineLog", 0, PipelineLog, ISerializable)

PipelineLog::PipelineLog()
:	m_level(0)
{
}

PipelineLog::PipelineLog(int32_t level, const std::wstring& text)
:	m_level(level)
,	m_text(text)
{
}

void PipelineLog::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"level", m_level);
	s >> Member< std::wstring >(L"text", m_text);
}


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

	virtual void log(int32_t level, const std::wstring& str)
	{
		if (m_originalTarget)
			m_originalTarget->log(level, str);

		if (m_transport->connected())
		{
			const PipelineLog tlog(level, str);
			m_transport->send(&tlog);
		}
	}

private:
	Ref< ILogTarget > m_originalTarget;
	Ref< net::BidirectionalObjectTransport > m_transport;
};



bool perform(const PipelineParameters* params)
{
	if (!FileSystem::getInstance().setCurrentVolumeAndDirectory(params->getWorkingDirectory()))
	{
		traktor::log::error << L"Unable to change working directory" << Endl;
		return false;
	}

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

	std::set< std::wstring > modules = settings->getProperty< PropertyStringSet >(L"Editor.Modules");
	for (std::set< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
	{
		Library library;
		if (!library.open(*i))
		{
			traktor::log::error << L"Unable to load module \"" << *i << L"\"" << Endl;
			return false;
		}

		library.detach();
	}

	std::wstring sourceDatabaseCS = settings->getProperty< PropertyString >(L"Editor.SourceDatabase");
	std::wstring outputDatabaseCS = settings->getProperty< PropertyString >(L"Editor.OutputDatabase");
	
	Ref< db::Database > sourceDatabase = openDatabase(sourceDatabaseCS, false);
	if (!sourceDatabase)
	{
		traktor::log::error << L"Unable to open source database \"" << sourceDatabaseCS << L"\"" << Endl;
		return false;
	}

	Ref< db::Database > outputDatabase = openDatabase(outputDatabaseCS, true);
	if (!outputDatabase)
	{
		traktor::log::error << L"Unable to open or create output database \"" << outputDatabaseCS << L"\"" << Endl;
		return false;
	}

	std::wstring pipelineDbConnectionStr = settings->getProperty< PropertyString >(L"Pipeline.Db");

	Ref< editor::PipelineDb > pipelineDb = new editor::PipelineDb();
	if (!pipelineDb->open(pipelineDbConnectionStr))
	{
		traktor::log::error << L"Unable to connect to pipeline database" << Endl;
		return false;
	}

	// Create cache if enabled.
	Ref< editor::IPipelineCache > pipelineCache;
	if (settings->getProperty< PropertyBoolean >(L"Pipeline.MemCached", false))
	{
		pipelineCache = new editor::MemCachedPipelineCache();
		if (!pipelineCache->create(settings))
		{
			traktor::log::warning << L"Unable to create pipeline cache; cache disabled" << Endl;
			pipelineCache = 0;
		}
	}
	if (settings->getProperty< PropertyBoolean >(L"Pipeline.FileCache", false))
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
	//if (settings->getProperty< PropertyBoolean >(L"Pipeline.BuildThreads", true))
	//{
	//	pipelineDepends = new editor::PipelineDependsParallel(
	//		&pipelineFactory,
	//		sourceDatabase,
	//		outputDatabase,
	//		&pipelineDependencySet,
	//		0
	//	);
	//}
	//else
	{
		pipelineDepends = new editor::PipelineDependsIncremental(
			&pipelineFactory,
			sourceDatabase,
			outputDatabase,
			&pipelineDependencySet
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
		db::recursiveFindChildInstances(sourceDatabase->getRootGroup(), db::FindInstanceByType(type_of< editor::Assets >()), assetInstances);
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
		sourceDatabase,
		outputDatabase,
		pipelineCache,
		pipelineDb,
		statusListener.ptr(),
		settings->getProperty< PropertyBoolean >(L"Pipeline.BuildThreads", true)
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

	traktor::log::info << L"Waiting..." << Endl;

	net::TcpSocket socket;
	if (!socket.bind(net::SocketAddressIPv4(52100)))
	{
		traktor::log::error << L"Unable to bind socket to port 52100" << Endl;
		return 1;
	}

	if (!socket.listen())
	{
		traktor::log::error << L"Unable to listen on socket" << Endl;
		return 1;
	}

	while (!g_receivedBreakSignal)
	{
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

		if (params)
			perform(params);
		else
			traktor::log::error << L"Unable to read pipeline parameters" << Endl;

		traktor::log::info   .setGlobalTarget(infoTarget);
		traktor::log::warning.setGlobalTarget(warningTarget);
		traktor::log::error  .setGlobalTarget(errorTarget);

		transport->close();
		transport = 0;
	}

	traktor::log::info << L"Bye" << Endl;
	return 0;
}

int master(const CommandLine& cmdLine)
{
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

	if (!g_pipelineMutex.existing())
	{
		// Get full path to our executable.
		TCHAR szFileName[MAX_PATH];
		if (!GetModuleFileName(NULL, szFileName, sizeof(szFileName)))
			return 1;

		// Spawn slave process.
		Ref< IProcess > slaveProcess = OS::getInstance().execute(
			tstows(szFileName),
			L"-slave",
			L"",
			&OS::getInstance().getEnvironment(),
			false,
			false,
			true
		);

		if (!slaveProcess)
			return 1;
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

	Ref< net::TcpSocket > socket = new net::TcpSocket();
	if (!socket->connect(net::SocketAddressIPv4(L"localhost", 52100)))
	{
		traktor::log::error << L"Unable to establish connection with pipeline slave" << Endl;
		return 1;
	}

	socket->setNoDelay(true);

	std::wstring settingsFile = L"Traktor.Editor";
	if (cmdLine.hasOption('s', L"settings"))
		settingsFile = cmdLine.getOption('s', L"settings").getString();

	PipelineParameters params(
		FileSystem::getInstance().getAbsolutePath(L"").getPathName(),
		settingsFile,
		cmdLine.hasOption('p', L"progress"),
		cmdLine.hasOption('f', L"force"),
		cmdLine.hasOption('n', L"no-cache"),
		roots
	);

	Ref< net::BidirectionalObjectTransport > transport = new net::BidirectionalObjectTransport(socket);

	transport->send(&params);
	for (;;)
	{
		Ref< PipelineLog > plog;
		if (transport->recv< PipelineLog >(1000, plog) == net::BidirectionalObjectTransport::RtDisconnected)
			break;

		if (plog)
		{
			switch (plog->getLevel())
			{
			default:
			case 0:
				traktor::log::info << plog->getText() << Endl;
				break;
			case 1:
				traktor::log::warning << plog->getText() << Endl;
				break;
			case 2:
				traktor::log::error << plog->getText() << Endl;
				break;
			case 3:
				traktor::log::debug << plog->getText() << Endl;
				break;
			}
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

	return 0;
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
		cmdLine.hasOption('p', L"progress"),
		cmdLine.hasOption('f', L"force"),
		cmdLine.hasOption('n', L"no-cache"),
		roots
	);

	perform(&params);

	traktor::log::info << L"Bye" << Endl;
	return 0;
}

int main(int argc, const char** argv)
{
	int32_t result = 0;

	net::Network::initialize();

	CommandLine cmdLine(argc, argv);
	if (cmdLine.hasOption(L"slave"))
		result = slave(cmdLine);
	else if (cmdLine.hasOption(L"standalone"))
		result = standalone(cmdLine);
	else
		result = master(cmdLine);

	net::Network::finalize();

	return result;
}
