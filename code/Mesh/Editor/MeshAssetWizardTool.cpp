#include "Mesh/Editor/MeshAssetWizardTool.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Formats/ModelFormat.h"
#include "Model/Model.h"
#include "Render/ShaderGraph.h"
#include "Editor/IEditor.h"
#include "Editor/IProject.h"
#include "Editor/Settings.h"
#include "Database/Database.h"
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

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.mesh.MeshAssetWizardTool", MeshAssetWizardTool, editor::IWizardTool)

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

	// Optionally create materials.
	Guid defaultMaterial = editor->getSettings()->getProperty< editor::PropertyGuid >(L"MeshPipeline.DefaultMaterial");
	if (defaultMaterial.isValid() && !defaultMaterial.isNull())
	{
		if (ui::MessageBox::show(
			i18n::Text(L"MESHASSET_WIZARDTOOL_CREATE_MATERIALS"),
			i18n::Text(L"MESHASSET_WIZARDTOOL_CREATE_MATERIALS_TITLE"),
			ui::MbIconQuestion | ui::MbYesNo
		) == ui::DrYes)
		{
			Ref< render::ShaderGraph > defaultMaterialShader = editor->getProject()->getSourceDatabase()->getObjectReadOnly< render::ShaderGraph >(defaultMaterial);
			if (!defaultMaterialShader)
			{
				log::error << L"Mesh asset wizard failed; unable to open default material shader" << Endl;
				return false;
			}

			Ref< model::Model > model = model::ModelFormat::readAny(fileName, model::ModelFormat::IfMaterials);
			if (!model)
			{
				log::error << L"Mesh asset wizard failed; unable to read source model" << Endl;
				return false;
			}

			const std::vector< model::Material >& materials = model->getMaterials();
			for (std::vector< model::Material >::const_iterator i = materials.begin(); i != materials.end(); ++i)
			{
				Ref< db::Instance > materialInstance = group->createInstance(i->getName());
				if (!materialInstance)
				{
					log::error << L"Mesh asset wizard failed; unable to create material instance" << Endl;
					return false;
				}

				materialInstance->setObject(defaultMaterialShader);
				if (!materialInstance->commit())
				{
					log::error << L"Mesh asset wizard failed; unable to commit material instance" << Endl;
					return false;
				}
			}
		}
	}

	// Create source asset.
	Ref< MeshAsset > asset = gc_new< MeshAsset >();
	asset->setFileName(fileName);
	asset->m_meshType = MeshAsset::MtStatic;

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
