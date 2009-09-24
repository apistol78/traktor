#include "Editor/PipelineManager.h"
#include "Editor/PipelineHash.h"
#include "Editor/IPipeline.h"
#include "Editor/MemCachedPipelineCache.h"
#include "Editor/Settings.h"
#include "Editor/Assets.h"
#include "Database/Local/LocalDatabase.h"
#include "Database/Compact/CompactDatabase.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Xml/XmlDeserializer.h"
#include "Core/Library/Library.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Stream.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/System/OS.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

using namespace traktor;

db::Database* openDatabase(const std::wstring& databaseName, bool create)
{
	Ref< db::IProviderDatabase > providerDatabase;

	if (endsWith(toLower(databaseName), L".manifest"))
	{
		Ref< db::LocalDatabase > localDatabase = gc_new< db::LocalDatabase >();
		if (!localDatabase->open(databaseName))
		{
			if (!create || !localDatabase->create(databaseName))
				return 0;
		}

		providerDatabase = localDatabase;
	}
	else if (endsWith(toLower(databaseName), L".compact"))
	{
		Ref< db::CompactDatabase > compactDatabase = gc_new< db::CompactDatabase >();
		if (!compactDatabase->open(databaseName))
		{
			if (!create || !compactDatabase->create(databaseName))
				return 0;
		}

		providerDatabase = compactDatabase;
	}

	T_ASSERT (providerDatabase);

	Ref< db::Database > database = gc_new< db::Database >();
	return database->create(providerDatabase) ? database.getPtr() : 0;
}

editor::Settings* loadSettings(const std::wstring& settingsFile)
{
	Ref< editor::PropertyGroup > globalGroup, userGroup;
	Ref< Stream > file;

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
		userGroup = gc_new< editor::PropertyGroup >();

	return gc_new< editor::Settings >(globalGroup, userGroup);
}

editor::PipelineHash* loadPipelineHash(const std::wstring& pipelineHashFile)
{
	Ref< editor::PipelineHash > pipelineHash;
	Ref< Stream > file = FileSystem::getInstance().open(pipelineHashFile, File::FmRead);
	if (file)
	{
		pipelineHash = BinarySerializer(file).readObject< editor::PipelineHash >();
		file->close();
	}
	return pipelineHash;
}

void savePipelineHash(const std::wstring& pipelineHashFile, editor::PipelineHash* pipelineHash)
{
	Ref< Stream > file = FileSystem::getInstance().open(pipelineHashFile, File::FmWrite);
	if (file)
	{
		BinarySerializer(file).writeObject(pipelineHash);
		file->close();
	}
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

	traktor::log::info << L"Loading pipeline modules..." << Endl;
	traktor::log::info << IncreaseIndent;

	std::vector< std::wstring > modules = settings->getProperty< editor::PropertyStringArray >(L"Editor.Modules");
	for (std::vector< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
	{
		traktor::log::info << *i << L"..." << Endl;
		if (!Library().open(*i))
		{
			traktor::log::error << L"Unable to load pipeline module \"" << *i << L"\"" << Endl;
			return 6;
		}
	}

	traktor::log::info << DecreaseIndent;

	std::wstring sourceManifest = settings->getProperty< editor::PropertyString >(L"Editor.SourceManifest");
	
	Ref< db::Database > sourceDatabase = openDatabase(sourceManifest, false);
	if (!sourceDatabase)
	{
		traktor::log::error << L"Unable to open source database \"" << sourceManifest << L"\"" << Endl;
		return 1;
	}

	std::wstring outputManifest = settings->getProperty< editor::PropertyString >(L"Editor.OutputManifest");
	
	Ref< db::Database > outputDatabase = openDatabase(outputManifest, true);
	if (!outputDatabase)
	{
		traktor::log::error << L"Unable to open or create output database \"" << outputManifest << L"\"" << Endl;
		return 2;
	}

	std::wstring hashFile = settings->getProperty< editor::PropertyString >(L"Pipeline.Hash");

	Ref< editor::PipelineHash > pipelineHash;
	if (!hashFile.empty())
		pipelineHash = loadPipelineHash(hashFile);
	if (!pipelineHash)
		pipelineHash = gc_new< editor::PipelineHash >();

	RefArray< editor::IPipeline > pipelines;

	std::vector< const Type* > pipelineTypes;
	type_of< editor::IPipeline >().findAllOf(pipelineTypes);

	for (std::vector< const Type* >::iterator i = pipelineTypes.begin(); i != pipelineTypes.end(); ++i)
	{
		Ref< editor::IPipeline > pipeline = dynamic_type_cast< editor::IPipeline* >((*i)->newInstance());
		if (!pipeline)
			continue;

		if (!pipeline->create(settings))
			continue;

		pipelines.push_back(pipeline);
	}

	Ref< editor::IPipelineCache > pipelineCache = gc_new< editor::MemCachedPipelineCache >();
	if (!pipelineCache->create(settings))
	{
		traktor::log::error << L"Unable to create pipeline cache; cache disabled" << Endl;
		pipelineCache = 0;
	}

	editor::PipelineManager pipelineManager(
		sourceDatabase,
		outputDatabase,
		pipelineCache,
		pipelines,
		pipelineHash
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
			pipelineManager.addDependency(assetGuid, true);
		}
	}
	else
	{
		RefArray< db::Instance > assetInstances;
		db::recursiveFindChildInstances(sourceDatabase->getRootGroup(), db::FindInstanceByType(type_of< editor::Assets >()), assetInstances);

		for (RefArray< db::Instance >::iterator i = assetInstances.begin(); i != assetInstances.end(); ++i)
			pipelineManager.addDependency(*i, true);
	}

	traktor::log::info << DecreaseIndent;

	bool rebuild = cmdLine.hasOption('f');
	if (!rebuild)
		traktor::log::info << L"Building assets..." << Endl;
	else
		traktor::log::info << L"Rebuilding assets..." << Endl;

	traktor::log::info << IncreaseIndent;

	pipelineManager.build(rebuild);

	traktor::log::info << DecreaseIndent;
	traktor::log::info << L"Finished" << Endl;

	if (!hashFile.empty())
		savePipelineHash(hashFile, pipelineHash);

	outputDatabase->close();
	sourceDatabase->close();

	for (RefArray< editor::IPipeline >::iterator i = pipelines.begin(); i != pipelines.end(); ++i)
		(*i)->destroy();

	return 0;
}
