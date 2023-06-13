/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/System/OS.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Database/Group.h"
#include "Editor/IEditor.h"
#include "I18N/Format.h"
#include "I18N/Text.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Physics/Editor/Material.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/Editor/MeshAssetEditor.h"
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/CheckBox.h"
#include "Ui/Container.h"
#include "Ui/Edit.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Slider.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/DropDown.h"
#include "Ui/FileDialog.h"
#include "Ui/InputDialog.h"
#include "Ui/MessageBox.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.MeshAssetEditor", MeshAssetEditor, editor::IObjectEditor)

MeshAssetEditor::MeshAssetEditor(editor::IEditor* editor)
:	m_editor(editor)
{
	m_assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath");
	m_modelCachePath =  m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.ModelCache.Path");
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
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut)))
		return false;

	Ref< ui::Container > containerFile = new ui::Container();
	if (!containerFile->create(container, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 4_ut, 4_ut)))
		return false;

	Ref< ui::Static > staticFileName = new ui::Static();
	if (!staticFileName->create(containerFile, i18n::Text(L"PHYSICS_MESHASSET_EDITOR_FILENAME")))
		return false;

	Ref< ui::Container > containerFileName = new ui::Container();
	if (!containerFileName->create(containerFile, ui::WsNone, new ui::TableLayout(L"100%,*,*,*", L"*", 0_ut, 4_ut)))
		return false;

	m_editFileName = new ui::Edit();
	if (!m_editFileName->create(containerFileName, L""))
		return false;

	Ref< ui::Button > browseButton = new ui::Button();
	if (!browseButton->create(containerFileName, L"..."))
		return false;
	browseButton->addEventHandler< ui::ButtonClickEvent >(this, &MeshAssetEditor::eventBrowseClick);

	Ref< ui::Button > previewModelButton = new ui::Button();
	if (!previewModelButton->create(containerFileName, i18n::Text(L"PHYSICS_MESHASSET_EDITOR_PREVIEW_MESH")))
		return false;
	previewModelButton->addEventHandler< ui::ButtonClickEvent >(this, &MeshAssetEditor::eventPreviewModelClick);

	Ref< ui::Button > editModelButton = new ui::Button();
	if (!editModelButton->create(containerFileName, i18n::Text(L"PHYSICS_MESHASSET_EDITOR_EDIT_MESH")))
		return false;
	editModelButton->addEventHandler< ui::ButtonClickEvent >(this, &MeshAssetEditor::eventEditModelClick);

	Ref< ui::Static > staticDummy = new ui::Static();
	staticDummy->create(containerFile, L"");

	Ref< ui::Container > containerOptions = new ui::Container();
	containerOptions->create(containerFile, ui::WsNone, new ui::TableLayout(L"50%,50%", L"*", 0_ut, 0_ut));

	Ref< ui::Container > containerLeft = new ui::Container();
	containerLeft->create(containerOptions, ui::WsNone, new ui::TableLayout(L"*", L"*", 0_ut, 4_ut));

	m_checkHull = new ui::CheckBox();
	if (!m_checkHull->create(containerLeft, i18n::Text(L"PHYSICS_MESHASSET_EDITOR_HULL")))
		return false;

	Ref< ui::Container > containerRight = new ui::Container();
	containerRight->create(containerOptions, ui::WsNone, new ui::TableLayout(L"*,*", L"*", 0_ut, 4_ut));

	Ref< ui::Static > staticMargin = new ui::Static();
	staticMargin->create(containerRight, i18n::Text(L"PHYSICS_MESHASSET_EDITOR_MARGIN"));

	m_editMargin = new ui::Edit();
	m_editMargin->create(containerRight, L"", ui::WsNone, new ui::NumericEditValidator(true, 0.0f, 10000.0f, 2));

	m_containerMaterials = new ui::Container();
	if (!m_containerMaterials->create(container, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%,*,100%", 0_ut, 0_ut)))
		return false;

	Ref< ui::ToolBar > materialTools = new ui::ToolBar();
	if (!materialTools->create(m_containerMaterials))
		return false;

	materialTools->addItem(new ui::ToolBarButton(i18n::Text(L"PHYSICS_MESHASSET_EDITOR_BROWSE_MATERIAL"), ui::Command(L"MeshAssetEditor.BrowseMaterial")));
	materialTools->addItem(new ui::ToolBarButton(i18n::Text(L"PHYSICS_MESHASSET_EDITOR_REMOVE_MATERIAL"), ui::Command(L"MeshAssetEditor.RemoveMaterial")));
	materialTools->addEventHandler< ui::ToolBarButtonClickEvent >(this, &MeshAssetEditor::eventMaterialToolClick);

	m_materialList = new ui::GridView();
	if (!m_materialList->create(m_containerMaterials, ui::GridView::WsColumnHeader | ui::WsDoubleBuffer))
		return false;

	m_materialList->addColumn(new ui::GridColumn(i18n::Text(L"PHYSICS_MESHASSET_EDITOR_MATERIAL_NAME"), 180_ut));
	m_materialList->addColumn(new ui::GridColumn(i18n::Text(L"PHYSICS_MESHASSET_EDITOR_MATERIAL_INSTANCE"), 350_ut));
	m_materialList->addEventHandler< ui::MouseDoubleClickEvent >(this, &MeshAssetEditor::eventMaterialListDoubleClick);

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
	//m_asset->setImportFilter(m_editImportFilter->getText());
	m_asset->setCalculateConvexHull(m_checkHull->isChecked());
	m_asset->setMargin(parseString< float >(m_editMargin->getText()));

	std::map< std::wstring, Guid > materials;
	for (auto materialItem : m_materialList->getRows())
	{
		std::wstring materialName = materialItem->get(0)->getText();
		Ref< db::Instance > materialInstance = materialItem->getData< db::Instance >(L"MATERIAL");

		if (materialInstance)
			materials.insert(std::make_pair(materialName, materialInstance->getGuid()));
	}
	m_asset->setMaterials(materials);

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
		800,
		600
	);
}

void MeshAssetEditor::updateModel()
{
	Path assetPath = FileSystem::getInstance().getAbsolutePath(m_assetPath, m_asset->getFileName());
	std::wstring importFilter = L""; // m_asset->getImportFilter();
	m_model = model::ModelCache(m_modelCachePath).get(assetPath, importFilter);
}

void MeshAssetEditor::updateFile()
{
	Path assetPath = FileSystem::getInstance().getAbsolutePath(m_assetPath, m_asset->getFileName());

	Path assetRelPath;
	if (!FileSystem::getInstance().getRelativePath(assetPath, m_assetPath, assetRelPath))
		assetRelPath = m_asset->getFileName().getOriginal();

	m_editFileName->setText(assetRelPath.getPathName());
	m_checkHull->setChecked(m_asset->getCalculateConvexHull());
	m_editMargin->setText(toString(m_asset->getMargin()));
}

void MeshAssetEditor::updateMaterialList()
{
	m_materialList->removeAllRows();

	if (m_model)
	{
		const std::map< std::wstring, Guid >& materialTemplates = m_asset->getMaterials();
		for (const auto& material : m_model->getMaterials())
		{
			Ref< ui::GridRow > shaderItem = new ui::GridRow();
			shaderItem->add(new ui::GridItem(material.getName()));

			std::wstring materialName = i18n::Text(L"PHYSICS_MESHASSET_EDITOR_MATERIAL_NOT_ASSIGNED");

			{
				auto it = materialTemplates.find(material.getName());
				if (it != materialTemplates.end())
				{
					if (!it->second.isNull())
					{
						Ref< db::Instance > materialInstance = m_editor->getSourceDatabase()->getInstance(it->second);
						if (materialInstance)
						{
							materialName = materialInstance->getName();
							shaderItem->setData(L"MATERIAL", materialInstance);
						}
					}
					else
						materialName = i18n::Text(L"PHYSICS_MESHASSET_EDITOR_MATERIAL_DISABLED");
				}
			}

			shaderItem->add(new ui::GridItem(materialName));

			m_materialList->addRow(shaderItem);
		}

		m_containerMaterials->setEnable(true);
	}
	else
		m_containerMaterials->setEnable(false);
}

void MeshAssetEditor::browseMaterial()
{
	Ref< ui::GridRow > selectedItem = m_materialList->getSelectedRow();
	if (!selectedItem)
		return;

	Ref< db::Instance > materialTemplateInstance = m_editor->browseInstance(type_of< Material >());
	if (materialTemplateInstance)
	{
		selectedItem->set(1, new ui::GridItem(materialTemplateInstance->getName()));
		selectedItem->setData(L"MATERIAL", materialTemplateInstance);
		m_materialList->requestUpdate();
	}
}

void MeshAssetEditor::removeMaterial()
{
	Ref< ui::GridRow > selectedItem = m_materialList->getSelectedRow();
	if (!selectedItem)
		return;

	selectedItem->set(1, new ui::GridItem(i18n::Text(L"PHYSICS_MESHASSET_EDITOR_MATERIAL_NOT_ASSIGNED")));
	selectedItem->setData(L"MATERIAL", nullptr);
	m_materialList->requestUpdate();
}

void MeshAssetEditor::eventBrowseClick(ui::ButtonClickEvent* event)
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(m_editFileName, type_name(this), i18n::Text(L"EDITOR_BROWSE_FILE"), L"All files (*.*);*.*"))
		return;

	Path assetPath = FileSystem::getInstance().getAbsolutePath(m_assetPath);
	Path path = assetPath + Path(m_editFileName->getText());

	if (fileDialog.showModal(path) == ui::DialogResult::Ok)
	{
		// Try get path relative to asset path.
		Path relPath;
		if (FileSystem::getInstance().getRelativePath(path, assetPath, relPath))
			path = relPath;

		m_editFileName->setText(path.getOriginal());
		m_asset->setFileName(path);

		updateModel();
		updateFile();
		updateMaterialList();
	}

	fileDialog.destroy();
}

void MeshAssetEditor::eventPreviewModelClick(ui::ButtonClickEvent* event)
{
	const std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");

	Ref< PropertyGroup > params = new PropertyGroup();
	params->setProperty< PropertyString >(L"fileName", assetPath + L"/" + m_editFileName->getText());
	params->setProperty< PropertyFloat >(L"scale", 1.0f/*m_asset->getScaleFactor()*/);

	m_editor->openTool(L"traktor.model.ModelTool", params);
}

void MeshAssetEditor::eventEditModelClick(ui::ButtonClickEvent* event)
{
	const std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	const Path path = FileSystem::getInstance().getAbsolutePath(Path(assetPath + L"/" + m_editFileName->getText()));
	OS::getInstance().openFile(path.getPathName());
}

void MeshAssetEditor::eventMaterialToolClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (command == L"MeshAssetEditor.BrowseMaterial")
		browseMaterial();
	else if (command == L"MeshAssetEditor.RemoveMaterial")
		removeMaterial();
}

void MeshAssetEditor::eventMaterialListDoubleClick(ui::MouseDoubleClickEvent* event)
{
	Ref< ui::GridRow > selectedItem = m_materialList->getSelectedRow();
	if (!selectedItem)
		return;

	const ui::Point mousePosition = event->getPosition();

	const int32_t column = m_materialList->getColumnIndex(mousePosition.x);
	if (column == 1)
	{
		Ref< db::Instance > materialInstance = selectedItem->getData< db::Instance >(L"MATERIAL");
		if (!materialInstance)
			browseMaterial();
		else
			m_editor->openEditor(materialInstance);
	}
}

}
