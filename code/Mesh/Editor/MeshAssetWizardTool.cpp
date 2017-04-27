/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
#include "Ui/FileDialog.h"
#include "Ui/MessageBox.h"

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
	// Select source model.
	ui::FileDialog fileDialog;
	if (!fileDialog.create(parent, i18n::Text(L"MESHASSET_WIZARDTOOL_FILE_TITLE"), L"All files;*.*"))
		return 0;

	Path fileName;
	if (fileDialog.showModal(fileName) != ui::DrOk)
	{
		fileDialog.destroy();
		return true;
	}
	fileDialog.destroy();

	// Create path relative to asset path.
	std::wstring assetPath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
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
