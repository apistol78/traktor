#include "Core/RefSet.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/ConnectionString.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/Asset.h"
#include "Editor/IEditor.h"
#include "Editor/IPipelineDependencySet.h"
#include "Editor/PipelineDependency.h"
#include "Net/Url.h"
#include "Net/Http/HttpClient.h"
#include "Net/Http/HttpClientResult.h"
#include "Net/Http/HttpRequestContent.h"
#include "Store/Editor/PublishWizardTool.h"

namespace traktor
{
	namespace store
	{
		namespace
		{

bool isSystemInstance(db::Instance* instance)
{
	for (db::Group* group = instance->getParent(); group; group = group->getParent())
	{
		if (group->getName() == L"System")
			return true;
	}
	return false;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.store.ScriptClassWizardTool", 0, PublishWizardTool, editor::IWizardTool)

std::wstring PublishWizardTool::getDescription() const
{
	return L"STORE_PUBLISH_WIZARDTOOL_DESCRIPTION";
}

uint32_t PublishWizardTool::getFlags() const
{
	return editor::IWizardTool::WfInstance;
}

bool PublishWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	std::wstring serverHost = editor->getSettings()->getProperty< std::wstring >(L"Store.Server", L"127.0.0.1:8118");
	if (serverHost.empty())
		return false;

	// Ensure temporary folder for upload exist.
	if (!FileSystem::getInstance().makeAllDirectories(Path(L"$(TRAKTOR_HOME)/data/Temp/Store/Upload")))
		return false;

	// Create our temporary working directory.
	RefSet< db::Instance > instances;
	std::set< Path > files;

	// Get array of instances, including all dependencies.
	auto object = instance->getObject();
	if (!object)
	{
		log::error << L"Publish failed; unable to read object of instance \"" << instance->getGuid().format() << L"\"." << Endl;
		return false;
	}

	auto dependencySet = editor->buildAssetDependencies(object, ~0);
	for (uint32_t i = 0; i < dependencySet->size(); ++i)
	{
		auto dependency = dependencySet->get(i);
		auto dependencyInstance = editor->getSourceDatabase()->getInstance(dependency->sourceInstanceGuid);
		if (!dependencyInstance)
		{
			log::warning << L"Unable to read dependency instance \"" << dependency->sourceInstanceGuid.format() << L"\"." << Endl;
			continue;
		}

		if (isSystemInstance(instance) || !isSystemInstance(dependencyInstance))
		{
			for (const auto& file : dependency->files)
				files.insert(file.filePath);

			instances.insert(dependencyInstance);
		}
	}

	instances.insert(instance);

	// Create a compact database with all selected instances migrated.
	Ref< db::Database > database = new db::Database();
	if (!database->create(db::ConnectionString(L"provider=traktor.db.CompactDatabase;fileName=$(TRAKTOR_HOME)/data/Temp/Store/Upload/Database.compact")))
	{
		log::error << L"Publish failed; unable to create bundle database." << Endl;
		return false;
	}
	for (const auto instance : instances)
	{
		auto object = instance->getObject();
		auto guid = instance->getGuid();

		auto migrateInstance = database->createInstance(
			L"Instances/" + instance->getPath(),
			db::CifDefault,
			&guid
		);
		if (!migrateInstance)
		{
			log::error << L"Publish failed; failed to create migration instance." << Endl;
			return false;
		}

		if (!migrateInstance->setObject(object))
		{
			log::error << L"Publish failed; failed to set object in migration instance." << Endl;
			return false;
		}

		std::vector< std::wstring > dataNames;
		instance->getDataNames(dataNames);
		for (const auto& dataName : dataNames)
		{
			auto source = instance->readData(dataName);
			auto destination = migrateInstance->writeData(dataName);
			if (!StreamCopy(destination, source).execute())
			{
				log::error << L"Publish failed; failed to write data stream into migration instance." << Endl;
				return false;
			}
			destination->close();
			source->close();
		}

		if (!migrateInstance->commit())
		{
			log::error << L"Publish failed; failed to commit migration instance." << Endl;
			return false;
		}
	}

	// Embed files.
	Path assetPath = FileSystem::getInstance().getAbsolutePath(Path(L"data/Assets"));
	for (const auto& file : files)
	{
		Path sourceFile;
		FileSystem::getInstance().getRelativePath(file, assetPath, sourceFile);

		Ref< db::Instance > assetInstance = database->createInstance(L"Assets/" + sourceFile.getPathName());
		assetInstance->setObject(new editor::Asset(sourceFile));

		Ref< IStream > assetStream = assetInstance->writeData(L"Data");
		if (!assetStream)
			return false;

		Ref< IStream > fileStream = FileSystem::getInstance().open(file, File::FmRead);
		if (!fileStream)
			return false;

		if (!StreamCopy(assetStream, fileStream).execute())
			return false;

		assetInstance->commit();
	}

	database->close();
	database = nullptr;

	// Upload files.
	{
		StringOutputStream os;

		os << L"<?xml version=\"1.0\"?>" << Endl;
		os << L"<manifest>" << Endl;
		os << L"\t<name>" << instance->getName() << L"</name>" << Endl;
		os << L"\t<description>" << instance->getName() << L"</description>" << Endl;
		os << L"\t<author>" << Endl;
		os << L"\t\t<name/>" << Endl;
		os << L"\t\t<e-mail>anders.pistol@doctorentertainment.com</e-mail>" << Endl;
		os << L"\t\t<phone/>" << Endl;
		os << L"\t\t<site/>" << Endl;
		os << L"\t</author>" << Endl;
		os << L"\t<thumbnail-url/>" << Endl;
		os << L"\t<database-url>Database.compact</database-url>" << Endl;
		os << L"</manifest>" << Endl;

		Guid categoryId = Guid(L"{0516941b-6d18-418d-95bc-f76d64bf8ed9}");
		Guid packageId = Guid::create();

		// Upload manifest.
		Ref< net::HttpClient > httpClient = new net::HttpClient();
		auto uploadManifest = httpClient->put(
			net::Url(L"http://" + serverHost + L"/" + categoryId.format() + L"/" + packageId.format() + L"/Manifest.xml"),
			new net::HttpRequestContent(os.str())
		);
		if (!uploadManifest || !uploadManifest->succeeded())
			return false;

		// Upload database.
		auto fileStream = FileSystem::getInstance().open(Path(L"$(TRAKTOR_HOME)/data/Temp/Store/Upload/Database.compact"), File::FmRead);

		auto uploadDatabase = httpClient->put(
			net::Url(L"http://" + serverHost + L"/" + categoryId.format() + L"/" + packageId.format() + L"/Database.compact"),
			new net::HttpRequestContent(fileStream)
		);
		if (!uploadDatabase || !uploadDatabase->succeeded())
			return false;

		fileStream->close();
		fileStream = nullptr;
	}

	log::info << L"Published successfully." << Endl;
	return true;
}

	}
}