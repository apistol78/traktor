/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/RefSet.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Database/ConnectionString.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/Asset.h"
#include "Editor/IEditor.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/PipelineDependencySet.h"
#include "Editor/PipelineDependency.h"
#include "I18N/Text.h"
#include "Store/Editor/PublishWizardTool.h"
#include "Ui/FileDialog.h"

namespace traktor::store
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.store.PublishWizardTool", 0, PublishWizardTool, editor::IWizardTool)

std::wstring PublishWizardTool::getDescription() const
{
	return i18n::Text(L"STORE_PUBLISH_WIZARDTOOL_DESCRIPTION");
}

const TypeInfoSet PublishWizardTool::getSupportedTypes() const
{
	return TypeInfoSet();
}

uint32_t PublishWizardTool::getFlags() const
{
	return editor::IWizardTool::WfInstance;
}

bool PublishWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	const Path assetPath = FileSystem::getInstance().getAbsolutePath(Path(L"data/Assets"));
	Path fileName;

	ui::FileDialog saveAsDialog;
	saveAsDialog.create(parent, L"", L"Save package as...", L"Package files (*.compact);*.compact;All files (*.*);*.*", L"", true);
	if (saveAsDialog.showModalThenDestroy(fileName) != ui::DialogResult::Ok)
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

	editor::PipelineDependencySet dependencySet;
	Ref< editor::IPipelineDepends > depends = editor->createPipelineDepends(&dependencySet, std::numeric_limits< uint32_t >::max());
	if (!depends)
		return false;

	depends->addDependency(object);
	depends->waitUntilFinished();

	for (uint32_t i = 0; i < dependencySet.size(); ++i)
	{
		auto dependency = dependencySet.get(i);
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
	if (!database->create(db::ConnectionString(L"provider=traktor.db.CompactDatabase;fileName=" + fileName.getPathName())))
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

		AlignedVector< std::wstring > dataNames;
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

	log::info << L"Published successfully." << Endl;
	return true;
}

}
