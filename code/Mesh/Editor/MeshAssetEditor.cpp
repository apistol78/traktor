#include <algorithm>
#include "Mesh/Editor/MeshAssetEditor.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MaterialShaderGenerator.h"
#include "Editor/IEditor.h"
#include "Editor/IProject.h"
#include "Editor/TypeBrowseFilter.h"
#include "Editor/Settings.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Database/Group.h"
#include "Model/Formats/ModelFormat.h"
#include "Model/Model.h"
#include "Render/ShaderGraph.h"
#include "I18N/Text.h"
#include "Ui/MethodHandler.h"
#include "Ui/TableLayout.h"
#include "Ui/Container.h"
#include "Ui/DropDown.h"
#include "Ui/ListView.h"
#include "Ui/ListViewItems.h"
#include "Ui/ListViewItem.h"
#include "Ui/Edit.h"
#include "Ui/FileDialog.h"
#include "Ui/Static.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/MiniButton.h"
#include "Core/Heap/GcNew.h"
#include "Core/Io/FileSystem.h"

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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshAssetEditor", MeshAssetEditor, editor::IObjectEditor)

MeshAssetEditor::MeshAssetEditor(editor::IEditor* editor)
:	m_editor(editor)
{
	m_assetPath = m_editor->getSettings()->getProperty< editor::PropertyString >(L"Pipeline.AssetPath", L"");
}

bool MeshAssetEditor::create(ui::Widget* parent, db::Instance* instance, Serializable* object)
{
	m_instance = instance;
	if (!m_instance)
		return false;

	m_asset = dynamic_type_cast< MeshAsset* >(object);
	if (!m_asset)
		return false;

	Ref< ui::Container > container = gc_new< ui::Container >();
	if (!container->create(parent, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"*,100%", 0, 0)))
		return false;

	Ref< ui::Container > containerFile = gc_new< ui::Container >();
	if (!containerFile->create(container, ui::WsNone, gc_new< ui::TableLayout >(L"*,100%", L"*", 4, 4)))
		return false;

	Ref< ui::Static > staticFileName = gc_new< ui::Static >();
	if (!staticFileName->create(containerFile, i18n::Text(L"MESHASSET_EDITOR_FILENAME")))
		return false;

	Ref< ui::Container > containerFileName = gc_new< ui::Container >();
	if (!containerFileName->create(containerFile, ui::WsNone, gc_new< ui::TableLayout >(L"100%,*", L"*", 0, 0)))
		return false;

	m_editFileName = gc_new< ui::Edit >();
	if (!m_editFileName->create(containerFileName, L""))
		return false;

	Ref< ui::custom::MiniButton > browseButton = gc_new< ui::custom::MiniButton >();
	if (!browseButton->create(containerFileName, L"..."))
		return false;

	browseButton->addClickEventHandler(ui::createMethodHandler(this, &MeshAssetEditor::eventBrowseClick));

	Ref< ui::Static > staticMeshType = gc_new< ui::Static >();
	if (!staticMeshType->create(containerFile, i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE")))
		return false;

	m_dropMeshType = gc_new< ui::DropDown >();
	if (!m_dropMeshType->create(containerFile))
		return false;

	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_BLEND"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_INDOOR"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_INSTANCE"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_SKINNED"));
	m_dropMeshType->add(i18n::Text(L"MESHASSET_EDITOR_MESH_TYPE_STATIC"));

	m_containerMaterials = gc_new< ui::Container >();
	if (!m_containerMaterials->create(container, ui::WsClientBorder, gc_new< ui::TableLayout >(L"100%", L"*,100%", 0, 0)))
		return false;

	Ref< ui::custom::ToolBar > materialTools = gc_new< ui::custom::ToolBar >();
	if (!materialTools->create(m_containerMaterials))
		return false;

	materialTools->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"MESHASSET_EDITOR_CREATE_SHADER"), ui::Command(L"MeshAssetEditor.CreateShader"), 0, ui::custom::ToolBarButton::BsText));
	materialTools->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"MESHASSET_EDITOR_BROWSE_SHADER"), ui::Command(L"MeshAssetEditor.BrowseShader"), 0, ui::custom::ToolBarButton::BsText));
	materialTools->addItem(gc_new< ui::custom::ToolBarButton >(i18n::Text(L"MESHASSET_EDITOR_REMOVE_SHADER"), ui::Command(L"MeshAssetEditor.RemoveShader"), 0, ui::custom::ToolBarButton::BsText));
	materialTools->addClickEventHandler(ui::createMethodHandler(this, &MeshAssetEditor::eventMaterialToolClick));

	m_materialList = gc_new< ui::ListView >();
	if (!m_materialList->create(m_containerMaterials, ui::ListView::WsReport))
		return false;

	m_materialList->addColumn(i18n::Text(L"MESHASSET_EDITOR_MATERIAL"), 180);
	m_materialList->addColumn(i18n::Text(L"MESHASSET_EDITOR_SHADER"), 300);

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

	std::map< std::wstring, Guid > materialShaders;

	Ref< ui::ListViewItems > items = m_materialList->getItems();
	for (int i = 0; i < items->count(); ++i)
	{
		Ref< ui::ListViewItem > item = items->get(i);
		T_ASSERT (item);

		std::wstring materialName = item->getText(0);
		Ref< db::Instance > materialShaderInstance = item->getData< db::Instance >(L"INSTANCE");

		if (materialShaderInstance)
			materialShaders.insert(std::make_pair(materialName, materialShaderInstance->getGuid()));
	}

	m_asset->setMaterialShaders(materialShaders);
}

void MeshAssetEditor::updateModel()
{
	m_model = model::ModelFormat::readAny(
		m_asset->getFileName(),
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
}

void MeshAssetEditor::updateMaterialList()
{
	Ref< ui::ListViewItems > items;

	if (m_model)
	{
		const std::map< std::wstring, Guid >& materialShaders = m_asset->getMaterialShaders();

		items = gc_new< ui::ListViewItems >();

		const std::vector< model::Material >& materials = m_model->getMaterials();
		for (std::vector< model::Material >::const_iterator i = materials.begin(); i != materials.end(); ++i)
		{
			Ref< db::Instance > materialShaderInstance;
			std::wstring materialShader = i18n::Text(L"MESHASSET_EDITOR_NOT_ASSIGNED");

			std::map< std::wstring, Guid >::const_iterator it = materialShaders.find(i->getName());
			if (it != materialShaders.end())
			{
				materialShaderInstance = m_editor->getProject()->getSourceDatabase()->getInstance(it->second);
				if (materialShaderInstance)
					materialShader = materialShaderInstance->getName();
			}

			Ref< ui::ListViewItem > item = gc_new< ui::ListViewItem >();
			item->setText(0, i->getName());
			item->setText(1, materialShader);
			item->setData(L"INSTANCE", materialShaderInstance);
			items->add(item);
		}

		m_containerMaterials->setEnable(true);
	}
	else
		m_containerMaterials->setEnable(false);

	m_materialList->setItems(items);
}

void MeshAssetEditor::createMaterialShader()
{
	Ref< ui::ListViewItem > selectedItem = m_materialList->getSelectedItem();
	if (!selectedItem)
		return;

	std::wstring materialName = selectedItem->getText(0);

	const std::vector< model::Material >& materials = m_model->getMaterials();
	std::vector< model::Material >::const_iterator it = std::find_if(materials.begin(), materials.end(), FindMaterialPred(materialName));
	if (it == materials.end())
		return;

	MaterialShaderGenerator generator(
		m_editor->getProject()->getSourceDatabase(),
		m_model
	);
	Ref< render::ShaderGraph > materialShader = generator.generate(*it);
	if (materialShader)
	{
		Ref< db::Instance > materialShaderInstance = m_instance->getParent()->createInstance(materialName);
		if (materialShaderInstance)
		{
			materialShaderInstance->setObject(materialShader);
			if (materialShaderInstance->commit())
			{
				selectedItem->setText(1, materialShaderInstance->getName());
				selectedItem->setData(L"INSTANCE", materialShaderInstance);

				// @fixme
				m_materialList->setItems(m_materialList->getItems());
			}
		}
	}
}

void MeshAssetEditor::browseMaterialShader()
{
	Ref< ui::ListViewItem > selectedItem = m_materialList->getSelectedItem();
	if (!selectedItem)
		return;

	editor::TypeBrowseFilter filter(type_of< render::ShaderGraph >());
	Ref< db::Instance > materialShaderInstance = m_editor->browseInstance(&filter);
	if (materialShaderInstance)
	{
		selectedItem->setText(1, materialShaderInstance->getName());
		selectedItem->setData(L"INSTANCE", materialShaderInstance);

		// @fixme
		m_materialList->setItems(m_materialList->getItems());
	}
}

void MeshAssetEditor::removeMaterialShader()
{
	Ref< ui::ListViewItem > selectedItem = m_materialList->getSelectedItem();
	if (!selectedItem)
		return;

	selectedItem->setText(1, i18n::Text(L"MESHASSET_EDITOR_NOT_ASSIGNED"));
	selectedItem->setData(L"INSTANCE", 0);

	// @fixme
	m_materialList->setItems(m_materialList->getItems());
}

void MeshAssetEditor::eventBrowseClick(ui::Event* event)
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

void MeshAssetEditor::eventMaterialToolClick(ui::Event* event)
{
	const ui::CommandEvent* commandEvent = checked_type_cast< const ui::CommandEvent* >(event);
	const ui::Command& command = commandEvent->getCommand();

	if (command == L"MeshAssetEditor.CreateShader")
		createMaterialShader();
	else if (command == L"MeshAssetEditor.BrowseShader")
		browseMaterialShader();
	else if (command == L"MeshAssetEditor.RemoveShader")
		removeMaterialShader();
}

	}
}
