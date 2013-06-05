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
		std::wcout << L":" << index << L":" << count << std::endl;
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
	if (settings->getProperty< PropertyBoolean >(L"Pipeline.BuildThreads", true))
	{
		pipelineDepends = new editor::PipelineDependsParallel(
			&pipelineFactory,
			sourceDatabase,
			&pipelineDependencySet,
			0
		);
	}
	else
	{
		pipelineDepends = new editor::PipelineDependsIncremental(
			&pipelineFactory,
			sourceDatabase,
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


int slave(const CommandLine& cmdLine)
{
	traktor::log::info << L"Entering slave mode..." << Endl;

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

	traktor::log::info << L"Waiting for master..." << Endl;

	for (;;)
	{
		if (socket.select(true, false, false, 1000) <= 0)
			continue;

		Ref< net::TcpSocket > client = socket.accept();
		if (!client)
			continue;

		Ref< net::BidirectionalObjectTransport > transport = new net::BidirectionalObjectTransport(client);

		Ref< ILogTarget > infoTarget    = log::info.   getGlobalTarget();
		Ref< ILogTarget > warningTarget = log::warning.getGlobalTarget();
		Ref< ILogTarget > errorTarget   = log::error.  getGlobalTarget();

		log::info   .setGlobalTarget(new LogRedirect(infoTarget,    transport));
		log::warning.setGlobalTarget(new LogRedirect(warningTarget, transport));
		log::error  .setGlobalTarget(new LogRedirect(errorTarget,   transport));

		Ref< PipelineParameters > params;
		transport->recv< PipelineParameters >(1000, params);

		if (params)
			perform(params);
		else
			traktor::log::error << L"Unable to read pipeline parameters" << Endl;

		log::info   .setGlobalTarget(infoTarget);
		log::warning.setGlobalTarget(warningTarget);
		log::error  .setGlobalTarget(errorTarget);

		transport->close();
		transport = 0;
	}

	return 0;
}

int master(const CommandLine& cmdLine)
{
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

	net::TcpSocket socket;
	if (!socket.connect(net::SocketAddressIPv4(L"localhost", 52100)))
	{
		traktor::log::error << L"Unable to establish connection with pipeline slave" << Endl;
		return 1;
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

	Ref< net::BidirectionalObjectTransport > transport = new net::BidirectionalObjectTransport(&socket);

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
				log::info << plog->getText() << Endl;
				break;
			case 1:
				log::warning << plog->getText() << Endl;
				break;
			case 2:
				log::error << plog->getText() << Endl;
				break;
			case 3:
				log::debug << plog->getText() << Endl;
				break;
			}
		}
	}

	socket.close();
	return 0;
}

int main(int argc, const char** argv)
{
	int32_t result = 0;

	net::Network::initialize();

	CommandLine cmdLine(argc, argv);
	if (cmdLine.hasOption(L"slave"))
		result = slave(cmdLine);
	else
		result = master(cmdLine);

	net::Network::finalize();

	return result;
}
