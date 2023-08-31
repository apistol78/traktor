/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Group.h"
#include "Editor/IEditor.h"
#include "Sound/Editor/Tracker/ImportMod.h"
#include "Sound/Editor/Tracker/ImportSongWizardTool.h"
#include "Ui/FileDialog.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.ImportSongWizardTool", 0, ImportSongWizardTool, editor::IWizardTool)

std::wstring ImportSongWizardTool::getDescription() const
{
	return L"Import MOD/XM/S3M..."; // i18n::Text(L"IMPORT_SONG_WIZARDTOOL_DESCRIPTION");
}

const TypeInfoSet ImportSongWizardTool::getSupportedTypes() const
{
	return TypeInfoSet();
}

uint32_t ImportSongWizardTool::getFlags() const
{
	return WfGroup;
}

bool ImportSongWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	Path pathName;

	ui::FileDialog fileDialog;
	fileDialog.create(parent, type_name(this), L"Select MOD to import...", L"All files;*.*");
	fileDialog.showModal(pathName);
	fileDialog.destroy();

	std::wstring assetPath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	std::wstring samplePath = group->getPath();

	if (!FileSystem::getInstance().makeAllDirectories(assetPath + L"/" + samplePath))
		return false;

	ImportMod().import(pathName, assetPath, samplePath, group);

	return true;
}

	}
}
