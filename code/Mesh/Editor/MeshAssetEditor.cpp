#include <algorithm>
#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Database/Group.h"
#include "Editor/IEditor.h"
#include "I18N/Format.h"
#include "I18N/Text.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshAssetEditor.h"
#include "Mesh/Editor/MaterialShaderGenerator.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Render/ITexture.h"
#include "Render/Shader/ShaderGraph.h"
#include "Ui/Application.h"
#include "Ui/TableLayout.h"
#include "Ui/CheckBox.h"
#include "Ui/Container.h"
#include "Ui/DropDown.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Edit.h"
#include "Ui/FileDialog.h"
#include "Ui/Slider.h"
#include "Ui/Static.h"
#include "Ui/Custom/InputDialog.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

struct FindMaterialPred
{
	std::wstring m_name;

	FindMaterialPred(const std::wstring& name)
	:	m_name(name)
	{
	}

	bool operator () (const model::Material& material) const
	{
		return material.getName() == m_name;
	}
};

bool haveVertexColors(const model::Model& model)
{
	for (uint32_t i = 0; i < model.getVertexCount(); ++i)
	{
		if (model.getVertex(i).getColor() != model::c_InvalidIndex)
			return true;
	}
	return false;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshAssetEditor", MeshAssetEditor, editor::IObjectEditor)

MeshAssetEditor::MeshAssetEditor(editor::IEditor* editor)
:	m_editor(editor)
{
	m_assetPath = m_editor->getSettings()->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");
}

bool MeshAssetEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_instance = instance;
	if (!m_instance)
		return false;

	m_asset = dynamic_type_cast< MeshAsset* >(object);
	if (!m_asset)
		return false;

	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	Ref< ui::Container > containerFile = new ui::Container();
	if (!containerFile->create(container, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 4, 4)))
		return false;

	Ref< ui::Static > staticFileName = new ui::Static();
	if (!staticFileName->create(containerFile, i18n::Text(L"MESHASSET_EDITOR_FILENAME")))
		return false;

	Ref< ui::Container > containerFileName = new ui::Container();
	if (!containerFileName->create(containerFile, ui::WsNone, new ui::TableLayout(L"100%,*", L"*", 0, 0)))
		return false;

	m_editFileName = new ui::Edit();
	if (!m_editFileName->create(containerFileName, L""))
		return false;

	Ref< ui::custom::MiniButton > browseButton = new ui::custom::MiniButton();
	if (!browseButton->create(containerFileName, L"..."))
		return false;

	browseButton->addEventHandler< ui::ButtonClickEvent >(this, &MeshAssetEditor::eventBrowseClick);

	Ref< ui::Static > staticMeshType = new ui::Static();
	if (!staticMeshType->create(containerFile, i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE")))
		return false;

	m_dropMeshType = new ui::DropDown();
	if (!m_dropMeshType->create(containerFile))
		return false;

	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_BLEND"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_INDOOR"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_INSTANCE"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_LOD"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_PARTITION"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_SKINNED"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_STATIC"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_STREAM"));
	m_dropMeshType->addEventHandler< ui::SelectionChangeEvent >(this, &MeshAssetEditor::eventMeshTypeChange);

	Ref< ui::Static > staticDummy = new ui::Static();
	staticDummy->create(containerFile, L"");

	Ref< ui::Container > containerOptions = new ui::Container();
	containerOptions->create(containerFile, ui::WsNone, new ui::TableLayout(L"50%,50%", L"*", 0, 0));

	Ref< ui::Container > containerLeft = new ui::Container();
	containerLeft->create(containerOptions, ui::WsNone, new ui::TableLayout(L"*", L"*", 0, 4));
	
	m_checkBakeOcclusion = new ui::CheckBox();
	if (!m_checkBakeOcclusion->create(containerLeft, i18n::Text(L"MESHASSET_EDITOR_BAKE_OCCLUSION")))
		return false;

	m_checkCullDistantFaces = new ui::CheckBox();
	if (!m_checkCullDistantFaces->create(containerLeft, i18n::Text(L"MESHASSET_EDITOR_CULL_DISTANT_FACES")))
		return false;

	m_checkGenerateOccluder = new ui::CheckBox();
	if (!m_checkGenerateOccluder->create(containerLeft, i18n::Text(L"MESHASSET_EDITOR_GENERATE_OCCLUDER")))
		return false;

	Ref< ui::Container > containerRight = new ui::Container();
	containerRight->create(containerOptions, ui::WsNone, new ui::TableLayout(L"*,*", L"*", 0, 4));

	m_staticLodSteps = new ui::Static();
	m_staticLodSteps->create(containerRight, i18n::Format(L"MESHASSET_EDITOR_LOD_STEPS", 1));

	m_sliderLodSteps = new ui::Slider();
	m_sliderLodSteps->create(containerRight);
	m_sliderLodSteps->setRange(1, 64);
	m_sliderLodSteps->addEventHandler< ui::ContentChangeEvent >(this, &MeshAssetEditor::eventLodStepsChange);

	Ref< ui::Static > staticLodMaxDistance = new ui::Static();
	staticLodMaxDistance->create(containerRight, i18n::Text(L"MESHASSET_EDITOR_LOD_MAX_DISTANCE"));

	m_editLodMaxDistance = new ui::Edit();
	m_editLodMaxDistance->create(containerRight, L"", ui::WsClientBorder, new ui::NumericEditValidator(true, 0.0f, 10000.0f, 2));

	Ref< ui::Static > staticLodCullDistance = new ui::Static();
	staticLodCullDistance->create(containerRight, i18n::Text(L"MESHASSET_EDITOR_LOD_CULL_DISTANCE"));

	m_editLodCullDistance = new ui::Edit();
	m_editLodCullDistance->create(containerRight, L"", ui::WsClientBorder, new ui::NumericEditValidator(true, 0.0f, 10000.0f, 2));

	m_containerMaterials = new ui::Container();
	if (!m_containerMaterials->create(container, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%,*,100%", 0, 0)))
		return false;

	// Material shaders.
	Ref< ui::custom::ToolBar > materialShaderTools = new ui::custom::ToolBar();
	if (!materialShaderTools->create(m_containerMaterials))
		return false;

	materialShaderTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_BROWSE_TEMPLATE"), ui::Command(L"MeshAssetEditor.BrowseTemplate")));
	materialShaderTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_REMOVE_TEMPLATE"), ui::Command(L"MeshAssetEditor.RemoveTemplate")));
	materialShaderTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_CREATE_SHADER"), ui::Command(L"MeshAssetEditor.CreateShader")));
	materialShaderTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_BROWSE_SHADER"), ui::Command(L"MeshAssetEditor.BrowseShader")));
	materialShaderTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_REMOVE_SHADER"), ui::Command(L"MeshAssetEditor.RemoveShader")));
	materialShaderTools->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &MeshAssetEditor::eventMaterialShaderToolClick);

	m_materialShaderList = new ui::custom::GridView();
	if (!m_materialShaderList->create(m_containerMaterials, ui::custom::GridView::WsColumnHeader | ui::WsDoubleBuffer))
		return false;

	m_materialShaderList->addColumn(new ui::custom::GridColumn(i18n::Text(L"MESHASSET_EDITOR_MATERIAL"), ui::scaleBySystemDPI(180)));
	m_materialShaderList->addColumn(new ui::custom::GridColumn(i18n::Text(L"MESHASSET_EDITOR_TEMPLATE"), ui::scaleBySystemDPI(180)));
	m_materialShaderList->addColumn(new ui::custom::GridColumn(i18n::Text(L"MESHASSET_EDITOR_SHADER"), ui::scaleBySystemDPI(300)));
	m_materialShaderList->addEventHandler< ui::MouseDoubleClickEvent >(this, &MeshAssetEditor::eventMaterialShaderListDoubleClick);

	// Material textures.
	Ref< ui::custom::ToolBar > materialTextureTools = new ui::custom::ToolBar();
	if (!materialTextureTools->create(m_containerMaterials))
		return false;

	materialTextureTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_CREATE_TEXTURE"), ui::Command(L"MeshAssetEditor.CreateTexture")));
	materialTextureTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_BROWSE_TEXTURE"), ui::Command(L"MeshAssetEditor.BrowseTexture")));
	materialTextureTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_REMOVE_TEXTURE"), ui::Command(L"MeshAssetEditor.RemoveTexture")));
	materialTextureTools->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &MeshAssetEditor::eventMaterialTextureToolClick);

	m_materialTextureList = new ui::custom::GridView();
	if (!m_materialTextureList->create(m_containerMaterials, ui::custom::GridView::WsColumnHeader | ui::WsDoubleBuffer))
		return false;

	m_materialTextureList->addColumn(new ui::custom::GridColumn(i18n::Text(L"MESHASSET_EDITOR_TEXTURE_NAME"), ui::scaleBySystemDPI(180)));
	m_materialTextureList->addColumn(new ui::custom::GridColumn(i18n::Text(L"MESHASSET_EDITOR_TEXTURE_ASSET"), ui::scaleBySystemDPI(300)));
	m_materialTextureList->addEventHandler< ui::MouseDoubleClickEvent >(this, &MeshAssetEditor::eventMaterialTextureListDoubleClick);

	updateModel();
	updateFile();
	updateMaterialList();

	return true;
}

void MeshAssetEditor::destroy()
{
}

void MeshAssetEditor::apply()
{
	m_asset->setFileName(m_editFileName->getText());
	m_asset->setMeshType(MeshAsset::MeshType(m_dropMeshType->getSelected() + 1));
	m_asset->setBakeOcclusion(m_checkBakeOcclusion->isChecked());
	m_asset->setCullDistantFaces(m_checkCullDistantFaces->isChecked());
	m_asset->setGenerateOccluder(m_checkGenerateOccluder->isChecked());
	m_asset->setLodSteps(m_sliderLodSteps->getValue());
	m_asset->setLodMaxDistance(parseString< float >(m_editLodMaxDistance->getText()));
	m_asset->setLodCullDistance(parseString< float >(m_editLodCullDistance->getText()));

	std::map< std::wstring, Guid > materialTemplates;
	std::map< std::wstring, Guid > materialShaders;

	const RefArray< ui::custom::GridRow >& shaderItems = m_materialShaderList->getRows();
	for (RefArray< ui::custom::GridRow >::const_iterator i = shaderItems.begin(); i != shaderItems.end(); ++i)
	{
		Ref< ui::custom::GridRow > shaderItem = *i;
		T_ASSERT (shaderItem);

		std::wstring materialName = shaderItem->get(0)->getText();
		Ref< db::Instance > materialTemplateInstance = shaderItem->getData< db::Instance >(L"TEMPLATE");
		Ref< db::Instance > materialShaderInstance = shaderItem->getData< db::Instance >(L"INSTANCE");

		if (materialTemplateInstance)
			materialTemplates.insert(std::make_pair(materialName, materialTemplateInstance->getGuid()));
		if (materialShaderInstance)
			materialShaders.insert(std::make_pair(materialName, materialShaderInstance->getGuid()));
	}

	m_asset->setMaterialTemplates(materialTemplates);
	m_asset->setMaterialShaders(materialShaders);

	std::map< std::wstring, Guid > materialTextures;

	const RefArray< ui::custom::GridRow >& textureItems = m_materialTextureList->getRows();
	for (RefArray< ui::custom::GridRow >::const_iterator i = textureItems.begin(); i != textureItems.end(); ++i)
	{
		Ref< ui::custom::GridRow > textureItem = *i;
		T_ASSERT (textureItem);

		std::wstring textureName = textureItem->get(0)->getText();
		Ref< db::Instance > materialTextureInstance = textureItem->getData< db::Instance >(L"INSTANCE");

		if (materialTextureInstance)
			materialTextures.insert(std::make_pair(textureName, materialTextureInstance->getGuid()));
	}

	m_asset->setMaterialTextures(materialTextures);

	m_instance->setObject(m_asset);
}

bool MeshAssetEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void MeshAssetEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

ui::Size MeshAssetEditor::getPreferredSize() const
{
	return ui::Size(
		ui::scaleBySystemDPI(800),
		ui::scaleBySystemDPI(600)
	);
}

void MeshAssetEditor::updateModel()
{
	Path assetPath = FileSystem::getInstance().getAbsolutePath(m_assetPath, m_asset->getFileName());
	m_model = model::ModelFormat::readAny(
		assetPath,
		model::ModelFormat::IfMaterials
	);
}

void MeshAssetEditor::updateFile()
{
	Path assetPath = FileSystem::getInstance().getAbsolutePath(m_assetPath, m_asset->getFileName());
	
	Path assetRelPath;
	if (!FileSystem::getInstance().getRelativePath(assetPath, m_assetPath, assetRelPath))
		assetRelPath = m_asset->getFileName().getOriginal();

	m_editFileName->setText(assetRelPath.getPathName());
	m_dropMeshType->select(m_asset->getMeshType() - 1);
	m_checkBakeOcclusion->setChecked(m_asset->getBakeOcclusion());
	m_checkCullDistantFaces->setChecked(m_asset->getCullDistantFaces());
	m_checkGenerateOccluder->setChecked(m_asset->getGenerateOccluder());
	
	m_staticLodSteps->setText(i18n::Format(L"MESHASSET_EDITOR_LOD_STEPS", m_asset->getLodSteps()));

	m_sliderLodSteps->setEnable(m_asset->getMeshType() == MeshAsset::MtLod);
	m_sliderLodSteps->setValue(m_asset->getLodSteps());
	
	m_editLodMaxDistance->setEnable(m_asset->getMeshType() == MeshAsset::MtLod);
	m_editLodMaxDistance->setText(toString(m_asset->getLodMaxDistance()));
	
	m_editLodCullDistance->setEnable(m_asset->getMeshType() == MeshAsset::MtLod);
	m_editLodCullDistance->setText(toString(m_asset->getLodCullDistance()));
}

void MeshAssetEditor::updateMaterialList()
{
	m_materialShaderList->removeAllRows();
	m_materialTextureList->removeAllRows();

	if (m_model)
	{
		const std::vector< model::Material >& materials = m_model->getMaterials();

		const std::map< std::wstring, Guid >& materialTemplates = m_asset->getMaterialTemplates();
		const std::map< std::wstring, Guid >& materialShaders = m_asset->getMaterialShaders();

		for (std::vector< model::Material >::const_iterator i = materials.begin(); i != materials.end(); ++i)
		{
			Ref< ui::custom::GridRow > shaderItem = new ui::custom::GridRow();
			shaderItem->add(new ui::custom::GridItem(i->getName()));

			std::wstring materialTemplate = i18n::Text(L"MESHASSET_EDITOR_TEMPLATE_NOT_ASSIGNED");
			std::wstring materialShader = i18n::Text(L"MESHASSET_EDITOR_SHADER_NOT_ASSIGNED");

			std::map< std::wstring, Guid >::const_iterator it = materialTemplates.find(i->getName());
			if (it != materialTemplates.end())
			{
				if (!it->second.isNull())
				{
					Ref< db::Instance > materialTemplateInstance = m_editor->getSourceDatabase()->getInstance(it->second);
					if (materialTemplateInstance)
					{
						materialTemplate = materialTemplateInstance->getName();
						shaderItem->setData(L"TEMPLATE", materialTemplateInstance);
					}
				}
				else
					materialTemplate = i18n::Text(L"MESHASSET_EDITOR_DISABLED");
			}

			std::map< std::wstring, Guid >::const_iterator it2 = materialShaders.find(i->getName());
			if (it2 != materialShaders.end())
			{
				if (!it2->second.isNull())
				{
					Ref< db::Instance > materialShaderInstance = m_editor->getSourceDatabase()->getInstance(it2->second);
					if (materialShaderInstance)
					{
						materialShader = materialShaderInstance->getName();
						shaderItem->setData(L"INSTANCE", materialShaderInstance);
					}
				}
				else
					materialShader = i18n::Text(L"MESHASSET_EDITOR_DISABLED");
			}

			shaderItem->add(new ui::custom::GridItem(materialTemplate));
			shaderItem->add(new ui::custom::GridItem(materialShader));

			m_materialShaderList->addRow(shaderItem);
		}

		std::set< std::wstring > textureNames;
		const std::map< std::wstring, Guid >& materialTextures = m_asset->getMaterialTextures();
		for (std::vector< model::Material >::const_iterator i = materials.begin(); i != materials.end(); ++i)
		{
			std::wstring modelTextures[] =
			{
				i->getDiffuseMap().name,
				i->getSpecularMap().name,
				i->getTransparencyMap().name,
				i->getEmissiveMap().name,
				i->getReflectiveMap().name,
				i->getNormalMap().name
			};

			for (uint32_t j = 0; j < sizeof_array(modelTextures); ++j)
			{
				if (modelTextures[j].empty() || textureNames.find(modelTextures[j]) != textureNames.end())
					continue;

				textureNames.insert(modelTextures[j]);

				Ref< db::Instance > materialTextureInstance;
				std::wstring materialTexture = i18n::Text(L"MESHASSET_EDITOR_TEXTURE_NOT_ASSIGNED");

				std::map< std::wstring, Guid >::const_iterator it = materialTextures.find(modelTextures[j]);
				if (it != materialTextures.end())
				{
					materialTextureInstance = m_editor->getSourceDatabase()->getInstance(it->second);
					if (materialTextureInstance)
						materialTexture = materialTextureInstance->getName();
				}

				Ref< ui::custom::GridRow > textureItem = new ui::custom::GridRow();
				textureItem->add(new ui::custom::GridItem(modelTextures[j]));
				textureItem->add(new ui::custom::GridItem(materialTexture));
				textureItem->setData(L"INSTANCE", materialTextureInstance);
				m_materialTextureList->addRow(textureItem);
			}
		}

		m_containerMaterials->setEnable(true);
	}
	else
		m_containerMaterials->setEnable(false);
}

void MeshAssetEditor::browseMaterialTemplate()
{
	Ref< ui::custom::GridRow > selectedItem = m_materialShaderList->getSelectedRow();
	if (!selectedItem)
		return;

	Ref< db::Instance > materialTemplateInstance = m_editor->browseInstance(type_of< render::ShaderGraph >());
	if (materialTemplateInstance)
	{
		selectedItem->set(1, new ui::custom::GridItem(materialTemplateInstance->getName()));
		selectedItem->setData(L"TEMPLATE", materialTemplateInstance);
		m_materialShaderList->requestUpdate();
	}
}

void MeshAssetEditor::removeMaterialTemplate()
{
	Ref< ui::custom::GridRow > selectedItem = m_materialShaderList->getSelectedRow();
	if (!selectedItem)
		return;

	selectedItem->set(1, new ui::custom::GridItem(i18n::Text(L"MESHASSET_EDITOR_TEMPLATE_NOT_ASSIGNED")));
	selectedItem->setData(L"TEMPLATE", 0);
	m_materialShaderList->requestUpdate();
}

void MeshAssetEditor::createMaterialShader()
{
	Ref< ui::custom::GridRow > selectedItem = m_materialShaderList->getSelectedRow();
	if (!selectedItem)
		return;

	std::wstring materialName = selectedItem->get(0)->getText();

	// Find model material to associate shader with.
	const std::vector< model::Material >& materials = m_model->getMaterials();
	std::vector< model::Material >::const_iterator it = std::find_if(materials.begin(), materials.end(), FindMaterialPred(materialName));
	if (it == materials.end())
		return;

	// Query user about material name; default model's material name.
	ui::custom::InputDialog::Field materialNameField =
	{
		L"",
		materialName,
		0,
		0
	};

	ui::custom::InputDialog materialNameDialog;
	if (materialNameDialog.create(m_materialShaderList, i18n::Text(L"MESHASSET_EDITOR_ENTER_NAME"), i18n::Text(L"MESHASSET_EDITOR_ENTER_NAME"), &materialNameField, 1))
	{
		if (materialNameDialog.showModal() == ui::DrOk)
			materialName = materialNameField.value;
		else
			materialName.clear();

		materialNameDialog.destroy();
	}

	if (materialName.empty())
		return;

	const std::map< std::wstring, Guid >& materialTemplates = m_asset->getMaterialTemplates();

	Guid materialTemplate;
	std::map< std::wstring, Guid >::const_iterator it2 = materialTemplates.find(materialName);
	if (it2 != materialTemplates.end())
		materialTemplate = it2->second;

	Ref< render::ShaderGraph > materialShader = MaterialShaderGenerator().generate(
		m_editor->getSourceDatabase(),
		*it,
		materialTemplate,
		m_asset->getMaterialTextures(),
		haveVertexColors(*m_model)
	);
	if (materialShader)
	{
		Ref< db::Instance > materialShaderInstance = m_instance->getParent()->createInstance(materialName);
		if (materialShaderInstance)
		{
			materialShaderInstance->setObject(materialShader);
			if (materialShaderInstance->commit())
			{
				selectedItem->set(2, new ui::custom::GridItem(materialShaderInstance->getName()));
				selectedItem->setData(L"INSTANCE", materialShaderInstance);
				m_materialShaderList->requestUpdate();
				m_editor->updateDatabaseView();
			}
		}
	}
}

void MeshAssetEditor::browseMaterialShader()
{
	Ref< ui::custom::GridRow > selectedItem = m_materialShaderList->getSelectedRow();
	if (!selectedItem)
		return;

	Ref< db::Instance > materialShaderInstance = m_editor->browseInstance(type_of< render::ShaderGraph >());
	if (materialShaderInstance)
	{
		selectedItem->set(2, new ui::custom::GridItem(materialShaderInstance->getName()));
		selectedItem->setData(L"INSTANCE", materialShaderInstance);
		m_materialShaderList->requestUpdate();
	}
}

void MeshAssetEditor::removeMaterialShader()
{
	Ref< ui::custom::GridRow > selectedItem = m_materialShaderList->getSelectedRow();
	if (!selectedItem)
		return;

	selectedItem->set(2, new ui::custom::GridItem(i18n::Text(L"MESHASSET_EDITOR_SHADER_NOT_ASSIGNED")));
	selectedItem->setData(L"INSTANCE", 0);
	m_materialShaderList->requestUpdate();
}

void MeshAssetEditor::createMaterialTexture()
{
}

void MeshAssetEditor::browseMaterialTexture()
{
	Ref< ui::custom::GridRow > selectedItem = m_materialTextureList->getSelectedRow();
	if (!selectedItem)
		return;

	Ref< db::Instance > materialTextureInstance = m_editor->browseInstance(type_of< render::ITexture >());
	if (materialTextureInstance)
	{
		selectedItem->set(1, new ui::custom::GridItem(materialTextureInstance->getName()));
		selectedItem->setData(L"INSTANCE", materialTextureInstance);
		m_materialTextureList->requestUpdate();
	}
}

void MeshAssetEditor::removeMaterialTexture()
{
	Ref< ui::custom::GridRow > selectedItem = m_materialTextureList->getSelectedRow();
	if (!selectedItem)
		return;

	selectedItem->set(1, new ui::custom::GridItem(i18n::Text(L"MESHASSET_EDITOR_TEXTURE_NOT_ASSIGNED")));
	selectedItem->setData(L"INSTANCE", 0);
	m_materialTextureList->requestUpdate();
}

void MeshAssetEditor::eventMeshTypeChange(ui::SelectionChangeEvent* event)
{
	updateFile();
}

void MeshAssetEditor::eventLodStepsChange(ui::ContentChangeEvent* event)
{
	m_staticLodSteps->setText(i18n::Format(L"MESHASSET_EDITOR_LOD_STEPS", m_sliderLodSteps->getValue()));
}

void MeshAssetEditor::eventBrowseClick(ui::ButtonClickEvent* event)
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(m_editFileName, i18n::Text(L"EDITOR_BROWSE_FILE"), L"All files (*.*);*.*"))
		return;

	Path path = m_editFileName->getText();
	if (fileDialog.showModal(path) == ui::DrOk)
	{
		m_editFileName->setText(path.getOriginal());
		m_asset->setFileName(path);

		updateModel();
		updateFile();
		updateMaterialList();
	}

	fileDialog.destroy();
}

void MeshAssetEditor::eventMaterialShaderToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (command == L"MeshAssetEditor.BrowseTemplate")
		browseMaterialTemplate();
	else if (command == L"MeshAssetEditor.RemoveTemplate")
		removeMaterialTemplate();
	else if (command == L"MeshAssetEditor.CreateShader")
		createMaterialShader();
	else if (command == L"MeshAssetEditor.BrowseShader")
		browseMaterialShader();
	else if (command == L"MeshAssetEditor.RemoveShader")
		removeMaterialShader();
}

void MeshAssetEditor::eventMaterialShaderListDoubleClick(ui::MouseDoubleClickEvent* event)
{
	Ref< ui::custom::GridRow > selectedItem = m_materialShaderList->getSelectedRow();
	if (!selectedItem)
		return;

	ui::Point mousePosition = event->getPosition();

	int32_t column = m_materialShaderList->getColumnIndex(mousePosition.x);
	if (column == 1)
	{
		// Template
		Ref< db::Instance > materialTemplateInstance = selectedItem->getData< db::Instance >(L"TEMPLATE");
		if (!materialTemplateInstance)
			browseMaterialTemplate();
		else
			m_editor->openEditor(materialTemplateInstance);
	}
	else if (column == 2)
	{
		// Shader
		Ref< db::Instance > materialShaderInstance = selectedItem->getData< db::Instance >(L"INSTANCE");
		if (!materialShaderInstance)
			browseMaterialShader();
		else
			m_editor->openEditor(materialShaderInstance);
	}
}

void MeshAssetEditor::eventMaterialTextureToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (command == L"MeshAssetEditor.CreateTexture")
		createMaterialTexture();
	else if (command == L"MeshAssetEditor.BrowseTexture")
		browseMaterialTexture();
	else if (command == L"MeshAssetEditor.RemoveTexture")
		removeMaterialTexture();
}

void MeshAssetEditor::eventMaterialTextureListDoubleClick(ui::MouseDoubleClickEvent* event)
{
	Ref< ui::custom::GridRow > selectedItem = m_materialTextureList->getSelectedRow();
	if (!selectedItem)
		return;

	ui::Point mousePosition = event->getPosition();

	int32_t column = m_materialTextureList->getColumnIndex(mousePosition.x);
	if (column == 1)
	{
		Ref< db::Instance > materialTextureInstance = selectedItem->getData< db::Instance >(L"INSTANCE");
		if (!materialTextureInstance)
			browseMaterialTexture();
		else
			m_editor->openEditor(materialTextureInstance);
	}
}

	}
}
