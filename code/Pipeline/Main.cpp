#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/System/OS.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Database/Compact/CompactDatabase.h"
#include "Database/Local/LocalDatabase.h"
#include "Editor/Assets.h"
#include "Editor/IPipeline.h"
#include "Editor/Settings.h"
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
		traktor::log::info << L":" << index << L":" << count << Endl;
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

Ref< editor::Settings > loadSettings(const std::wstring& settingsFile)
{
	Ref< editor::PropertyGroup > globalGroup, userGroup;
	Ref< traktor::IStream > file;

	std::wstring globalConfig = settingsFile + L".config";

	if ((file = FileSystem::getInstance().open(globalConfig, File::FmRead)) != 0)
	{
		globalGroup = dynamic_type_cast< editor::PropertyGroup* >(xml::XmlDeserializer(file).readObject());
		file->close();
	}

	if (!globalGroup)
		return 0;

	std::wstring userConfig = settingsFile + L"." + OS::getInstance().getCurrentUser() + L".config";

	if ((file = FileSystem::getInstance().open(userConfig, File::FmRead)) != 0)
	{
		userGroup = dynamic_type_cast< editor::PropertyGroup* >(xml::XmlDeserializer(file).readObject());
		file->close();
	}

	if (!userGroup)
		userGroup = new editor::PropertyGroup();

	return new editor::Settings(globalGroup, userGroup);
}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

#if defined(T_STATIC)
	T_FORCE_LINK_REF(editor::PropertyBoolean);
	T_FORCE_LINK_REF(editor::PropertyInteger);
	T_FORCE_LINK_REF(editor::PropertyFloat);
	T_FORCE_LINK_REF(editor::PropertyString);
	T_FORCE_LINK_REF(editor::PropertyStringArray);
	T_FORCE_LINK_REF(editor::PropertyGuidArray);
	T_FORCE_LINK_REF(editor::PropertyType);
	T_FORCE_LINK_REF(editor::PropertyTypeSet);
	T_FORCE_LINK_REF(editor::PropertyVector4);
	T_FORCE_LINK_REF(editor::PropertyQuaternion);
	T_FORCE_LINK_REF(editor::PropertyKey);
	T_FORCE_LINK_REF(editor::PropertyGroup);
	T_FORCE_LINK_REF(editor::PropertySerializable);
#endif

	if (cmdLine.hasOption('h'))
	{
		traktor::log::info << L"Usage: Traktor.Pipeline.App (-options) \"{Asset guid}\"" << Endl;
		traktor::log::info << L"       -f    Force rebuild" << Endl;
		traktor::log::info << L"       -s    Settings (default \"Traktor.Editor\")" << Endl;
		traktor::log::info << L"       -p    Write progress to stdout" << Endl;
		traktor::log::info << L"       -h    Show this help" << Endl;
		return 0;
	}

	std::wstring settingsFile = L"Traktor.Editor";
	if (cmdLine.hasOption('s'))
		settingsFile = cmdLine.getOption('s').getString();

	Ref< editor::Settings > settings = loadSettings(settingsFile);
	if (!settings)
	{
		traktor::log::error << L"Unable to load pipeline settings \"" << settingsFile << L"\"" << Endl;
		return 0;
	}

	std::vector< std::wstring > modules = settings->getProperty< editor::PropertyStringArray >(L"Editor.Modules");
	for (std::vector< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
	{
		if (!Library().open(*i))
		{
			traktor::log::error << L"Unable to load module \"" << *i << L"\"" << Endl;
			return 6;
		}
	}

	std::wstring sourceDatabaseCS = settings->getProperty< editor::PropertyString >(L"Editor.SourceDatabase");
	std::wstring outputDatabaseCS = settings->getProperty< editor::PropertyString >(L"Editor.OutputDatabase");
	
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

	std::wstring pipelineDbConnectionStr = settings->getProperty< editor::PropertyString >(L"Pipeline.Db");

	Ref< editor::PipelineDb > pipelineDb = new editor::PipelineDb();
	if (!pipelineDb->open(pipelineDbConnectionStr))
	{
		traktor::log::error << L"Unable to connect to pipeline database" << Endl;
		return 3;
	}

	// Create cache if enabled.
	Ref< editor::IPipelineCache > pipelineCache;
	if (settings->getProperty< editor::PropertyBoolean >(L"Pipeline.MemCached", false))
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

	return 0;
}
