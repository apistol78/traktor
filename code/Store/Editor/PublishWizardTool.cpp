#include "Core/RefSet.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Database/ConnectionString.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IPipelineDependencySet.h"
#include "Editor/PipelineDependency.h"
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
	// Create our temporary working directory.
	FileSystem::getInstance().makeAllDirectories(L"data/Temp/Publish");

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
	if (!database->create(db::ConnectionString(L"provider=traktor.db.CompactDatabase;fileName=data/Temp/Publish/Instances.compact")))
	{
		log::error << L"Publish failed; unable to create bundle database." << Endl;
		return false;
	}
	for (const auto instance : instances)
	{
		auto object = instance->getObject();
		auto guid = instance->getGuid();

		auto migrateInstance = database->createInstance(
			instance->getPath(),
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
	database->close();
	database = nullptr;

	// Copy files.
	Path assetPath = FileSystem::getInstance().getAbsolutePath(Path(L"data/Assets"));

	for (const auto& file : files)
	{
		Path sourceFile;
		FileSystem::getInstance().getRelativePath(file, assetPath, sourceFile);

		Path bundleFile = Path(L"data/Temp/Publish/Assets/" + sourceFile.getPathName());

		FileSystem::getInstance().makeAllDirectories(bundleFile.getPathOnly());

		FileSystem::getInstance().copy(
			bundleFile,
			file,
			true
		);
		
	}

	log::info << L"Published successfully." << Endl;
	return true;
}

	}
}