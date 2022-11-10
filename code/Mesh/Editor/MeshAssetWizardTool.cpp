/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshAssetWizardTool.h"
#include "Ui/MessageBox.h"
#include "Ui/FileDialog.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshAssetWizardTool", 0, MeshAssetWizardTool, editor::IWizardTool)

std::wstring MeshAssetWizardTool::getDescription() const
{
	return i18n::Text(L"MESHASSET_WIZARDTOOL_DESCRIPTION");
}

uint32_t MeshAssetWizardTool::getFlags() const
{
	return WfGroup;
}

bool MeshAssetWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	std::wstring assetPath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

	// Select source model.
	ui::FileDialog fileDialog;
	if (!fileDialog.create(parent, type_name(this), i18n::Text(L"MESHASSET_WIZARDTOOL_FILE_TITLE"), L"All files;*.*", assetPath))
		return false;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DialogResult::Ok)
	{
		fileDialog.destroy();
		return true;
	}
	fileDialog.destroy();

	// Create path relative to asset path.
	FileSystem::getInstance().getRelativePath(
		FileSystem::getInstance().getAbsolutePath(fileName),
		FileSystem::getInstance().getAbsolutePath(assetPath),
		fileName
	);

	// Create source asset.
	Ref< MeshAsset > asset = new MeshAsset();
	asset->setFileName(fileName);
	asset->setMeshType(MeshAsset::MtStatic);

	// Create asset instance.
	Ref< db::Instance > assetInstance = group->createInstance(
		fileName.getFileNameNoExtension(),
		db::CifReplaceExisting | db::CifKeepExistingGuid
	);
	if (!assetInstance)
	{
		log::error << L"Failed to create instance." << Endl;
		return false;
	}

	assetInstance->setObject(asset);

	if (!assetInstance->commit())
	{
		log::error << L"Failed to commit instance." << Endl;
		return false;
	}

	return true;
}

	}
}
