/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/Asset.h"
#include "Editor/IEditor.h"
#include "Editor/App/CheckAssetsTool.h"
#include "I18N/Text.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.CheckAssetsTool", 0, CheckAssetsTool, IEditorTool)

std::wstring CheckAssetsTool::getDescription() const
{
	return i18n::Text(L"EDITOR_CHECK_ASSETS");
}

Ref< ui::IBitmap > CheckAssetsTool::getIcon() const
{
	return nullptr;
}

bool CheckAssetsTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool CheckAssetsTool::launch(ui::Widget* parent, IEditor* editor, const PropertyGroup* param)
{
	Ref< db::Database > database = editor->getSourceDatabase();
	if (!database)
		return true;

	const std::wstring assetPath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

	RefArray< db::Instance > assetInstances;
	db::recursiveFindChildInstances(
		database->getRootGroup(),
		db::FindInstanceByType(type_of< Asset >()),
		assetInstances
	);

	int32_t errorCount = 0;

	for (auto assetInstance : assetInstances)
	{
		Ref< Asset > asset = assetInstance->getObject< Asset >();
		if (!asset)
		{
			log::error << L"Unable to read asset \"" << assetInstance->getPath() << L"\"" << Endl;
			++errorCount;
			continue;
		}

		if (!asset->getFileName().isRelative())
		{
			log::error << L"File \"" << asset->getFileName().getPathName() << L"\" absolute, in asset \"" << assetInstance->getPath() << L"\"" << Endl;
			++errorCount;
		}

		const Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, asset->getFileName());
		RefArray< File > files = FileSystem::getInstance().find(fileName);
		if (files.empty())
		{
			log::error << L"File \"" << fileName.getPathName() << L"\" missing, in asset \"" << assetInstance->getPath() << L"\"" << Endl;
			++errorCount;
		}
	}

	log::info << L"Check asset completed; " << errorCount << L" error(s) found in " << int32_t(assetInstances.size()) << L" asset(s)" << Endl;
	return true;
}

}
