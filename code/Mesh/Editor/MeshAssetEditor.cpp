#include <algorithm>
#include "Core/Io/FileSystem.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Database/Group.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshAssetEditor.h"
#include "Mesh/Editor/MaterialShaderGenerator.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Render/ITexture.h"
#include "Render/Shader/ShaderGraph.h"
#include "Ui/TableLayout.h"
#include "Ui/CheckBox.h"
#include "Ui/Container.h"
#include "Ui/DropDown.h"
#include "Ui/ListView.h"
#include "Ui/ListViewItems.h"
#include "Ui/ListViewItem.h"
#include "Ui/Edit.h"
#include "Ui/FileDialog.h"
#include "Ui/Static.h"
#include "Ui/Custom/InputDialog.h"
#include "Ui/Custom/MiniButton.h"
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
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_PARTITION"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_SKINNED"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_STATIC"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_STREAM"));
	
	Ref< ui::Static > staticDummy = new ui::Static();
	staticDummy->create(containerFile, L"");
	
	m_checkBakeOcclusion = new ui::CheckBox();
	if (!m_checkBakeOcclusion->create(containerFile, i18n::Text(L"MESHASSET_EDITOR_BAKE_OCCLUSION")))
		return false;

	Ref< ui::Static > staticDummy2 = new ui::Static();
	staticDummy2->create(containerFile, L"");

	m_checkGenerateOccluder = new ui::CheckBox();
	if (!m_checkGenerateOccluder->create(containerFile, i18n::Text(L"MESHASSET_EDITOR_GENERATE_OCCLUDER")))
		return false;

	m_containerMaterials = new ui::Container();
	if (!m_containerMaterials->create(container, ui::WsClientBorder, new ui::TableLayout(L"100%", L"*,100%,*,100%", 0, 0)))
		return false;

	// Material shaders.
	Ref< ui::custom::ToolBar > materialShaderTools = new ui::custom::ToolBar();
	if (!materialShaderTools->create(m_containerMaterials))
		return false;

	materialShaderTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_CREATE_SHADER"), ui::Command(L"MeshAssetEditor.CreateShader")));
	materialShaderTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_BROWSE_SHADER"), ui::Command(L"MeshAssetEditor.BrowseShader")));
	materialShaderTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_REMOVE_SHADER"), ui::Command(L"MeshAssetEditor.RemoveShader")));
	materialShaderTools->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &MeshAssetEditor::eventMaterialShaderToolClick);

	m_materialShaderList = new ui::ListView();
	if (!m_materialShaderList->create(m_containerMaterials, ui::ListView::WsReport))
		return false;

	m_materialShaderList->addColumn(i18n::Text(L"MESHASSET_EDITOR_MATERIAL"), 180);
	m_materialShaderList->addColumn(i18n::Text(L"MESHASSET_EDITOR_TEMPLATE"), 180);
	m_materialShaderList->addColumn(i18n::Text(L"MESHASSET_EDITOR_SHADER"), 300);
	m_materialShaderList->addEventHandler< ui::MouseDoubleClickEvent >(this, &MeshAssetEditor::eventMaterialShaderListDoubleClick);

	// Material textures.
	Ref< ui::custom::ToolBar > materialTextureTools = new ui::custom::ToolBar();
	if (!materialTextureTools->create(m_containerMaterials))
		return false;

	materialTextureTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_CREATE_TEXTURE"), ui::Command(L"MeshAssetEditor.CreateTexture")));
	materialTextureTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_BROWSE_TEXTURE"), ui::Command(L"MeshAssetEditor.BrowseTexture")));
	materialTextureTools->addItem(new ui::custom::ToolBarButton(i18n::Text(L"MESHASSET_EDITOR_REMOVE_TEXTURE"), ui::Command(L"MeshAssetEditor.RemoveTexture")));
	materialTextureTools->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &MeshAssetEditor::eventMaterialTextureToolClick);

	m_materialTextureList = new ui::ListView();
	if (!m_materialTextureList->create(m_containerMaterials, ui::ListView::WsReport))
		return false;

	m_materialTextureList->addColumn(i18n::Text(L"MESHASSET_EDITOR_TEXTURE_NAME"), 180);
	m_materialTextureList->addColumn(i18n::Text(L"MESHASSET_EDITOR_TEXTURE_ASSET"), 300);
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
	m_asset->setGenerateOccluder(m_checkGenerateOccluder->isChecked());

	Ref< ui::ListViewItems > shaderItems = m_materialShaderList->getItems();

	std::map< std::wstring, Guid > materialTemplates;
	std::map< std::wstring, Guid > materialShaders;
	if (shaderItems)
	{
		for (int i = 0; i < shaderItems->count(); ++i)
		{
			Ref< ui::ListViewItem > shaderItem = shaderItems->get(i);
			T_ASSERT (shaderItem);

			std::wstring materialName = shaderItem->getText(0);
			Ref< db::Instance > materialTemplateInstance = shaderItem->getData< db::Instance >(L"TEMPLATE");
			Ref< db::Instance > materialShaderInstance = shaderItem->getData< db::Instance >(L"INSTANCE");

			if (materialTemplateInstance)
				materialTemplates.insert(std::make_pair(materialName, materialTemplateInstance->getGuid()));
			if (materialShaderInstance)
				materialShaders.insert(std::make_pair(materialName, materialShaderInstance->getGuid()));
		}
	}
	m_asset->setMaterialTemplates(materialTemplates);
	m_asset->setMaterialShaders(materialShaders);

	std::map< std::wstring, Guid > materialTextures;
	Ref< ui::ListViewItems > textureItems = m_materialTextureList->getItems();
	if (textureItems)
	{
		for (int i = 0; i < textureItems->count(); ++i)
		{
			Ref< ui::ListViewItem > textureItem = textureItems->get(i);
			T_ASSERT (textureItem);

			std::wstring textureName = textureItem->getText(0);
			Ref< db::Instance > materialTextureInstance = textureItem->getData< db::Instance >(L"INSTANCE");

			if (materialTextureInstance)
				materialTextures.insert(std::make_pair(textureName, materialTextureInstance->getGuid()));
		}
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
	return ui::Size(800, 600);
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
	m_checkGenerateOccluder->setChecked(m_asset->getGenerateOccluder());
}

void MeshAssetEditor::updateMaterialList()
{
	Ref< ui::ListViewItems > shaderItems;
	Ref< ui::ListViewItems > textureItems;

	if (m_model)
	{
		const std::vector< model::Material >& materials = m_model->getMaterials();

		shaderItems = new ui::ListViewItems();

		const std::map< std::wstring, Guid >& materialTemplates = m_asset->getMaterialTemplates();
		const std::map< std::wstring, Guid >& materialShaders = m_asset->getMaterialShaders();

		for (std::vector< model::Material >::const_iterator i = materials.begin(); i != materials.end(); ++i)
		{
			Ref< ui::ListViewItem > shaderItem = new ui::ListViewItem();
			shaderItem->setText(0, i->getName());

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

			shaderItem->setText(1, materialTemplate);
			shaderItem->setText(2, materialShader);

			shaderItems->add(shaderItem);
		}

		textureItems = new ui::ListViewItems();

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

				Ref< ui::ListViewItem > textureItem = new ui::ListViewItem();
				textureItem->setText(0, modelTextures[j]);
				textureItem->setText(1, materialTexture);
				textureItem->setData(L"INSTANCE", materialTextureInstance);
				textureItems->add(textureItem);
			}
		}

		m_containerMaterials->setEnable(true);
	}
	else
		m_containerMaterials->setEnable(false);

	m_materialShaderList->setItems(shaderItems);
	m_materialTextureList->setItems(textureItems);
}

void MeshAssetEditor::browseMaterialTemplate()
{
	Ref< ui::ListViewItem > selectedItem = m_materialShaderList->getSelectedItem();
	if (!selectedItem)
		return;

	Ref< db::Instance > materialTemplateInstance = m_editor->browseInstance(type_of< render::ShaderGraph >());
	if (materialTemplateInstance)
	{
		selectedItem->setText(1, materialTemplateInstance->getName());
		selectedItem->setData(L"TEMPLATE", materialTemplateInstance);

		m_materialShaderList->setItems(m_materialShaderList->getItems());
	}
}

void MeshAssetEditor::removeMaterialTemplate()
{
	Ref< ui::ListViewItem > selectedItem = m_materialShaderList->getSelectedItem();
	if (!selectedItem)
		return;

	selectedItem->setText(1, i18n::Text(L"MESHASSET_EDITOR_TEMPLATE_NOT_ASSIGNED"));
	selectedItem->setData(L"TEMPLATE", 0);

	m_materialShaderList->setItems(m_materialShaderList->getItems());
}

void MeshAssetEditor::createMaterialShader()
{
	Ref< ui::ListViewItem > selectedItem = m_materialShaderList->getSelectedItem();
	if (!selectedItem)
		return;

	std::wstring materialName = selectedItem->getText(0);

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
				selectedItem->setText(2, materialShaderInstance->getName());
				selectedItem->setData(L"INSTANCE", materialShaderInstance);

				m_materialShaderList->setItems(m_materialShaderList->getItems());

				m_editor->updateDatabaseView();
			}
		}
	}
}

void MeshAssetEditor::browseMaterialShader()
{
	Ref< ui::ListViewItem > selectedItem = m_materialShaderList->getSelectedItem();
	if (!selectedItem)
		return;

	Ref< db::Instance > materialShaderInstance = m_editor->browseInstance(type_of< render::ShaderGraph >());
	if (materialShaderInstance)
	{
		selectedItem->setText(2, materialShaderInstance->getName());
		selectedItem->setData(L"INSTANCE", materialShaderInstance);

		m_materialShaderList->setItems(m_materialShaderList->getItems());
	}
}

void MeshAssetEditor::removeMaterialShader()
{
	Ref< ui::ListViewItem > selectedItem = m_materialShaderList->getSelectedItem();
	if (!selectedItem)
		return;

	selectedItem->setText(2, i18n::Text(L"MESHASSET_EDITOR_SHADER_NOT_ASSIGNED"));
	selectedItem->setData(L"INSTANCE", 0);

	m_materialShaderList->setItems(m_materialShaderList->getItems());
}

void MeshAssetEditor::createMaterialTexture()
{
}

void MeshAssetEditor::browseMaterialTexture()
{
	Ref< ui::ListViewItem > selectedItem = m_materialTextureList->getSelectedItem();
	if (!selectedItem)
		return;

	Ref< db::Instance > materialTextureInstance = m_editor->browseInstance(type_of< render::ITexture >());
	if (materialTextureInstance)
	{
		selectedItem->setText(1, materialTextureInstance->getName());
		selectedItem->setData(L"INSTANCE", materialTextureInstance);

		m_materialTextureList->setItems(m_materialTextureList->getItems());
	}
}

void MeshAssetEditor::removeMaterialTexture()
{
	Ref< ui::ListViewItem > selectedItem = m_materialTextureList->getSelectedItem();
	if (!selectedItem)
		return;

	selectedItem->setText(1, i18n::Text(L"MESHASSET_EDITOR_TEXTURE_NOT_ASSIGNED"));
	selectedItem->setData(L"INSTANCE", 0);

	m_materialTextureList->setItems(m_materialTextureList->getItems());
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
	if (command == L"MeshAssetEditor.CreateShader")
		createMaterialShader();
	else if (command == L"MeshAssetEditor.BrowseShader")
		browseMaterialShader();
	else if (command == L"MeshAssetEditor.RemoveShader")
		removeMaterialShader();
}

void MeshAssetEditor::eventMaterialShaderListDoubleClick(ui::MouseDoubleClickEvent* event)
{
	ui::Point mousePosition = event->getPosition();

	Ref< ui::ListViewItem > selectedItem = m_materialShaderList->getSelectedItem();
	if (!selectedItem)
		return;

	int32_t w0 = m_materialShaderList->getColumnWidth(0);
	int32_t w1 = m_materialShaderList->getColumnWidth(1);

	if (mousePosition.x <= w0 + w1)
	{
		// Template
		Ref< db::Instance > materialTemplateInstance = selectedItem->getData< db::Instance >(L"TEMPLATE");
		if (!materialTemplateInstance)
			browseMaterialTemplate();
		else
			m_editor->openEditor(materialTemplateInstance);
	}
	else if (mousePosition.x > w0 + w1)
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
	Ref< ui::ListViewItem > selectedItem = m_materialShaderList->getSelectedItem();
	if (!selectedItem)
		return;

	Ref< db::Instance > materialTextureInstance = selectedItem->getData< db::Instance >(L"INSTANCE");
	if (!materialTextureInstance)
		browseMaterialTexture();
	else
		m_editor->openEditor(materialTextureInstance);
}

	}
}
