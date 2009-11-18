#include "Mesh/Editor/MeshAssetWizardTool.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Editor/IEditor.h"
#include "Editor/IProject.h"
#include "Editor/Settings.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Ui/FileDialog.h"
#include "Ui/MessageBox.h"
#include "I18N/Text.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshAssetWizardTool", MeshAssetWizardTool, editor::IWizardTool)

std::wstring MeshAssetWizardTool::getDescription() const
{
	return i18n::Text(L"MESHASSET_WIZARDTOOL_DESCRIPTION");
}

bool MeshAssetWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group)
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
		return false;

	assetInstance->setObject(asset);

	if (!assetInstance->commit())
		return false;

	return true;
}

	}
}
