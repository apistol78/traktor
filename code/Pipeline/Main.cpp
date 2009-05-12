#include "Editor/PipelineManager.h"
#include "Editor/PipelineHash.h"
#include "Editor/Pipeline.h"
#include "Editor/Settings.h"
#include "Database/Local/LocalDatabase.h"
#include "Database/Compact/CompactDatabase.h"
#include "Database/Database.h"
#include "Xml/XmlDeserializer.h"
#include "Core/Library/Library.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Stream.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/System/OS.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/StringUtils.h"
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
		globalGroup = gc_new< editor::PropertyGroup >();

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

	if (cmdLine.getCount() < 1)
	{
		log::info << L"Usage: Traktor.Pipeline.App (-options) \"{Asset guid}\"" << Endl;
		log::info << L"       -f    Force rebuild" << Endl;
		log::info << L"       -s    Settings (default \"Traktor.Editor\")" << Endl;
		return 0;
	}

	std::wstring settingsFile = L"Traktor.Editor";
	if (cmdLine.hasOption('s'))
		settingsFile = cmdLine.getOption('s').getString();

	Ref< editor::Settings > settings = loadSettings(settingsFile);
	if (!settings)
	{
		log::error << L"Unable to load pipeline settings \"" << settingsFile << L"\"" << Endl;
		return 0;
	}

	log::info << L"Loading pipeline modules..." << Endl;
	log::info << IncreaseIndent;

	std::vector< std::wstring > modules = settings->getProperty< editor::PropertyStringArray >(L"Editor.Modules");
	for (std::vector< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
	{
		log::info << *i << L"..." << Endl;
		if (!Library().open(*i))
		{
			log::error << L"Unable to load pipeline module \"" << *i << L"\"" << Endl;
			return 6;
		}
	}

	log::info << DecreaseIndent;

	std::wstring sourceManifest = settings->getProperty< editor::PropertyString >(L"Editor.SourceManifest");
	
	Ref< db::Database > sourceDatabase = openDatabase(sourceManifest, false);
	if (!sourceDatabase)
	{
		log::error << L"Unable to open source database \"" << sourceManifest << L"\"" << Endl;
		return 1;
	}

	std::wstring outputManifest = settings->getProperty< editor::PropertyString >(L"Editor.OutputManifest");
	
	Ref< db::Database > outputDatabase = openDatabase(outputManifest, true);
	if (!outputDatabase)
	{
		log::error << L"Unable to open or create output database \"" << outputManifest << L"\"" << Endl;
		return 2;
	}

	std::wstring hashFile = settings->getProperty< editor::PropertyString >(L"Pipeline.Hash");

	Ref< editor::PipelineHash > pipelineHash;
	if (!hashFile.empty())
		pipelineHash = loadPipelineHash(hashFile);
	if (!pipelineHash)
		pipelineHash = gc_new< editor::PipelineHash >();

	RefArray< editor::Pipeline > pipelines;

	std::vector< const Type* > pipelineTypes;
	type_of< editor::Pipeline >().findAllOf(pipelineTypes);

	for (std::vector< const Type* >::iterator i = pipelineTypes.begin(); i != pipelineTypes.end(); ++i)
	{
		Ref< editor::Pipeline > pipeline = dynamic_type_cast< editor::Pipeline* >((*i)->newInstance());
		if (!pipeline)
			continue;

		if (!pipeline->create(settings))
			continue;

		pipelines.push_back(pipeline);
	}

	editor::PipelineManager pipelineManager(sourceDatabase, outputDatabase, pipelines, pipelineHash);

	log::info << L"Collecting dependencies..." << Endl;
	log::info << IncreaseIndent;

	Guid assetGuid(cmdLine.getString(0));
	if (assetGuid.isNull() || !assetGuid.isValid())
	{
		log::error << L"Invalid asset guid" << Endl;
		return 7;
	}

	pipelineManager.addDependency(assetGuid);

	log::info << DecreaseIndent;

	bool rebuild = cmdLine.hasOption('f');
	if (!rebuild)
		log::info << L"Building assets..." << Endl;
	else
		log::info << L"Rebuilding assets..." << Endl;

	log::info << IncreaseIndent;

	pipelineManager.build(rebuild);

	log::info << DecreaseIndent;
	log::info << L"Finished" << Endl;

	if (!hashFile.empty())
		savePipelineHash(hashFile, pipelineHash);

	outputDatabase->close();
	sourceDatabase->close();

	for (RefArray< editor::Pipeline >::iterator i = pipelines.begin(); i != pipelines.end(); ++i)
		(*i)->destroy();

	return 0;
}
