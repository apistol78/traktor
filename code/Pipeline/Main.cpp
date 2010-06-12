#include <iostream>
#include "Core/Io/FileSystem.h"
#include "Core/Io/FileOutputStreamBuffer.h"
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
#include "Core/Settings/PropertyStringArray.h"
#include "Core/Settings/Settings.h"
#include "Core/System/OS.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Database/Compact/CompactDatabase.h"
#include "Database/Local/LocalDatabase.h"
#include "Editor/Assets.h"
#include "Editor/IPipeline.h"
#include "Editor/Pipeline/MemCachedPipelineCache.h"
#include "Editor/Pipeline/PipelineBuilder.h"
#include "Editor/Pipeline/PipelineDb.h"
#include "Editor/Pipeline/PipelineDependsIncremental.h"
#include "Editor/Pipeline/PipelineFactory.h"
#include "Xml/XmlDeserializer.h"

using namespace traktor;

struct StatusListener : public editor::PipelineBuilder::IListener
{
	virtual void begunBuildingAsset(
		const std::wstring& assetName,
		uint32_t index,
		uint32_t count
	) const
	{
		std::wcout << L":" << index << L":" << count << std::endl;
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

Ref< Settings > loadSettings(const std::wstring& settingsFile)
{
	Ref< Settings > settings;
	Ref< IStream > file;

	std::wstring globalConfig = settingsFile + L".config";
	std::wstring userConfig = settingsFile + L"." + OS::getInstance().getCurrentUser() + L".config";

	if ((file = FileSystem::getInstance().open(userConfig, File::FmRead)) != 0)
	{
		settings = Settings::read< xml::XmlDeserializer >(file);
		file->close();
	}

	if (settings)
		return settings;

	if ((file = FileSystem::getInstance().open(globalConfig, File::FmRead)) != 0)
	{
		settings = Settings::read< xml::XmlDeserializer >(file);
		file->close();
	}

	return settings;
}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);
	Ref< traktor::IStream > logFile;

	if (cmdLine.hasOption('h'))
	{
		traktor::log::info << L"Usage: Traktor.Pipeline.App (-options) \"{Asset guid}\"" << Endl;
		traktor::log::info << L"       -f           Force rebuild" << Endl;
		traktor::log::info << L"       -s           Settings (default \"Traktor.Editor\")" << Endl;
		traktor::log::info << L"       -l=logfile   Save log file" << Endl;
		traktor::log::info << L"       -n           Disable memcached" << Endl;
		traktor::log::info << L"       -p           Write progress to stdout" << Endl;
		traktor::log::info << L"       -h           Show this help" << Endl;
		return 0;
	}

	if (cmdLine.hasOption('l'))
	{
		std::wstring logPath = cmdLine.getOption('l').getString();
		if ((logFile = FileSystem::getInstance().open(logPath, File::FmWrite)) != 0)
		{
			traktor::log::info   .setBuffer(new FileOutputStreamBuffer(logFile, new Utf8Encoding()));
			traktor::log::info   .setLineEnd(OutputStream::LeWin);
			traktor::log::warning.setBuffer(new FileOutputStreamBuffer(logFile, new Utf8Encoding()));
			traktor::log::warning.setLineEnd(OutputStream::LeWin);
			traktor::log::error  .setBuffer(new FileOutputStreamBuffer(logFile, new Utf8Encoding()));
			traktor::log::error  .setLineEnd(OutputStream::LeWin);
			traktor::log::debug  .setBuffer(new FileOutputStreamBuffer(logFile, new Utf8Encoding()));
			traktor::log::debug  .setLineEnd(OutputStream::LeWin);

			traktor::log::info << L"Log file \"Application.log\" created" << Endl;
		}
		else
			traktor::log::error << L"Unable to create log file; logging only to std pipes" << Endl;
	}

	std::wstring settingsFile = L"Traktor.Editor";
	if (cmdLine.hasOption('s'))
		settingsFile = cmdLine.getOption('s').getString();

	Ref< Settings > settings = loadSettings(settingsFile);
	if (!settings)
	{
		traktor::log::error << L"Unable to load pipeline settings \"" << settingsFile << L"\"" << Endl;
		return 0;
	}

	if (cmdLine.hasOption('n'))
		settings->setProperty< PropertyBoolean >(L"Pipeline.MemCached", false);

	std::vector< std::wstring > modules = settings->getProperty< PropertyStringArray >(L"Editor.Modules");
	for (std::vector< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
	{
		if (!Library().open(*i))
		{
			traktor::log::error << L"Unable to load module \"" << *i << L"\"" << Endl;
			return 6;
		}
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

	// Create pipeline factory.
	editor::PipelineFactory pipelineFactory(settings);

	// Collect dependencies.
	editor::PipelineDependsIncremental pipelineDepends(
		&pipelineFactory,
		sourceDatabase
	);

	traktor::log::info << L"Collecting dependencies..." << Endl;
	traktor::log::info << IncreaseIndent;

	if (cmdLine.getCount() > 0)
	{
		for (int32_t i = 0; i < cmdLine.getCount(); ++i)
		{
			Guid assetGuid(cmdLine.getString(i));
			if (assetGuid.isNull() || !assetGuid.isValid())
			{
				traktor::log::error << L"Invalid asset guid (" << i << L")" << Endl;
				return 7;
			}
			pipelineDepends.addDependency(assetGuid, editor::PdfBuild);
		}
	}
	else
	{
		RefArray< db::Instance > assetInstances;
		db::recursiveFindChildInstances(sourceDatabase->getRootGroup(), db::FindInstanceByType(type_of< editor::Assets >()), assetInstances);

		for (RefArray< db::Instance >::iterator i = assetInstances.begin(); i != assetInstances.end(); ++i)
			pipelineDepends.addDependency(*i, editor::PdfBuild);
	}

	traktor::log::info << DecreaseIndent;

	RefArray< editor::PipelineDependency > dependencies;
	pipelineDepends.getDependencies(dependencies);

	AutoPtr< StatusListener > statusListener;
	if (cmdLine.hasOption('p'))
		statusListener.reset(new StatusListener());

	// Build output.
	editor::PipelineBuilder pipelineBuilder(
		&pipelineFactory,
		sourceDatabase,
		outputDatabase,
		pipelineCache,
		pipelineDb,
		statusListener.ptr()
	);

	bool rebuild = cmdLine.hasOption('f');
	if (rebuild)
		traktor::log::info << L"Rebuilding " << uint32_t(dependencies.size()) << L" asset(s)..." << Endl;
	else
		traktor::log::info << L"Building " << uint32_t(dependencies.size()) << L" asset(s)..." << Endl;

	traktor::log::info << IncreaseIndent;

	pipelineBuilder.build(dependencies, rebuild);

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

	return 0;
}
