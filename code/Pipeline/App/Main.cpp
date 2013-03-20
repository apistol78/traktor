#if defined(_WIN32)
#	include <Windows.h>
#endif
#include <iostream>
#include "Core/Io/FileSystem.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/System/OS.h"
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
#include "Editor/Pipeline/PipelineDependsIncremental.h"
#include "Editor/Pipeline/PipelineDependsParallel.h"
#include "Editor/Pipeline/PipelineFactory.h"
#include "Xml/XmlDeserializer.h"

using namespace traktor;

class LogStreamTarget : public ILogTarget
{
public:
	LogStreamTarget(OutputStream* stream)
	:	m_stream(stream)
	{
	}

	virtual void log(const std::wstring& str)
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

	virtual void log(const std::wstring& str)
	{
		m_target1->log(str);
		m_target2->log(str);
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

Ref< db::Database > openDatabase(const std::wstring& connectionString, bool create)
{
	Ref< db::Database > database = new db::Database();
	if (!database->open(connectionString))
	{
		if (!create || !database->create(connectionString))
			return 0;
	}
	return database;
}

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

bool g_receivedBreakSignal = false;
bool g_success = false;

#if defined(_WIN32)

BOOL consoleCtrlHandler(DWORD fdwCtrlType)
{
	g_receivedBreakSignal = true;
	return TRUE;
}

#endif

void threadBuild(editor::PipelineBuilder& pipelineBuilder, const RefArray< editor::PipelineDependency >& dependencies, bool rebuild)
{
	g_success = pipelineBuilder.build(dependencies, rebuild);
}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);
	Ref< traktor::IStream > logFile;

	if (cmdLine.hasOption('h', L"help"))
	{
		traktor::log::info << L"Usage: Traktor.Pipeline.App (-options) \"{Asset guid}\"" << Endl;
		traktor::log::info << L"       -f|-force       Force rebuild" << Endl;
		traktor::log::info << L"       -s|-settings    Settings (default \"Traktor.Editor\")" << Endl;
		traktor::log::info << L"       -l|-log=logfile Save log file" << Endl;
		traktor::log::info << L"       -n|-no-cache    Disable cache" << Endl;
		traktor::log::info << L"       -p|-progress    Write progress to stdout" << Endl;
		traktor::log::info << L"       -h|-help        Show this help" << Endl;
		return 0;
	}

#if defined(_WIN32)
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)consoleCtrlHandler, TRUE);
#endif

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

	std::wstring settingsFile = L"Traktor.Editor";
	if (cmdLine.hasOption('s', L"settings"))
		settingsFile = cmdLine.getOption('s', L"settings").getString();

	Ref< PropertyGroup > settings = loadSettings(settingsFile);
	if (!settings)
	{
		traktor::log::error << L"Unable to load pipeline settings \"" << settingsFile << L"\"" << Endl;
		return 0;
	}

	if (cmdLine.hasOption('n', L"no-cache"))
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
			return 6;
		}
		library.detach();
	}

	std::wstring sourceDatabaseCS = settings->getProperty< PropertyString >(L"Editor.SourceDatabase");
	std::wstring outputDatabaseCS = settings->getProperty< PropertyString >(L"Editor.OutputDatabase");
	
	Ref< db::Database > sourceDatabase = openDatabase(sourceDatabaseCS, false);
	if (!sourceDatabase)
	{
		traktor::log::error << L"Unable to open source database \"" << sourceDatabaseCS << L"\"" << Endl;
		return 1;
	}

	Ref< db::Database > outputDatabase = openDatabase(outputDatabaseCS, true);
	if (!outputDatabase)
	{
		traktor::log::error << L"Unable to open or create output database \"" << outputDatabaseCS << L"\"" << Endl;
		return 2;
	}

	std::wstring pipelineDbConnectionStr = settings->getProperty< PropertyString >(L"Pipeline.Db");

	Ref< editor::PipelineDb > pipelineDb = new editor::PipelineDb();
	if (!pipelineDb->open(pipelineDbConnectionStr))
	{
		traktor::log::error << L"Unable to connect to pipeline database" << Endl;
		return 3;
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

	// Collect dependencies.
	Ref< editor::IPipelineDepends > pipelineDepends;
	if (settings->getProperty< PropertyBoolean >(L"Pipeline.BuildThreads", true))
	{
		pipelineDepends = new editor::PipelineDependsParallel(
			&pipelineFactory,
			0,
			sourceDatabase
		);
	}
	else
	{
		pipelineDepends = new editor::PipelineDependsIncremental(
			&pipelineFactory,
			0,
			sourceDatabase
		);
	}

	traktor::log::info << L"Collecting dependencies..." << Endl;
	traktor::log::info << IncreaseIndent;

	if (cmdLine.getCount() > 0)
	{
		for (int32_t i = 0; i < cmdLine.getCount(); ++i)
		{
			Guid assetGuid(cmdLine.getString(i));
			if (assetGuid.isNull() || !assetGuid.isValid())
			{
				traktor::log::error << L"Invalid root asset guid (" << i << L")" << Endl;
				return 7;
			}

			traktor::log::info << L"Traversing root \"" << assetGuid.format() << L"\"..." << Endl;
			pipelineDepends->addDependency(assetGuid, editor::PdfBuild);
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

	RefArray< editor::PipelineDependency > dependencies;
	pipelineDepends->getDependencies(dependencies);

	AutoPtr< StatusListener > statusListener;
	if (cmdLine.hasOption('p', L"progress"))
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

	bool rebuild = cmdLine.hasOption('f', L"force");
	if (rebuild)
		traktor::log::info << L"Rebuilding " << uint32_t(dependencies.size()) << L" asset(s)..." << Endl;
	else
		traktor::log::info << L"Building " << uint32_t(dependencies.size()) << L" asset(s)..." << Endl;

	traktor::log::info << IncreaseIndent;

	Thread* bt = ThreadManager::getInstance().create(
		makeStaticFunctor< editor::PipelineBuilder&, const RefArray< editor::PipelineDependency >&, bool >(&threadBuild, pipelineBuilder, dependencies, rebuild),
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

	pipelineDb->close();
	outputDatabase->close();
	sourceDatabase->close();

	if (logFile)
	{
		traktor::log::info.setBuffer(0);
		traktor::log::warning.setBuffer(0);
		traktor::log::error.setBuffer(0);
		traktor::log::debug.setBuffer(0);

		logFile->close();
		logFile = 0;
	}

	return g_success ? 0 : 8;
}
