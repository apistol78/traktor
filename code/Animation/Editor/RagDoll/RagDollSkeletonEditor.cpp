/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/RagDoll/RagDollSkeletonEditor.h"

#include "Animation/Editor/RagDoll/RagDollBone.h"
#include "Animation/Editor/RagDoll/RagDollPreviewControl.h"
#include "Animation/Editor/RagDoll/RagDollSkeletonAsset.h"
#include "Animation/Editor/SkeletonAsset.h"
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Core/Io/FileSystem.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/Operations/Transform.h"
#include "Render/IRenderSystem.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/Splitter.h"
#include "Ui/TableLayout.h"
#include "Ui/Events/MouseButtonDownEvent.h"
#include "Ui/PropertyList/ArrayPropertyItem.h"
#include "Ui/PropertyList/BrowsePropertyItem.h"
#include "Ui/PropertyList/ObjectPropertyItem.h"
#include "Ui/PropertyList/PropertyCommandEvent.h"
#include "Ui/PropertyList/PropertyContentChangeEvent.h"
#include "Ui/Events/SelectionChangeEvent.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewItem.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor::animation
{
	namespace
	{

// Label a bone in the tree by the animation joints it influences.
std::wstring boneLabel(const RagDollBone* bone)
{
	std::wstring label;
	for (const auto& influence : bone->getInfluences())
	{
		if (!label.empty())
			label += L", ";
		label += influence.joint;
	}
	return !label.empty() ? label : L"(bone)";
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RagDollSkeletonEditor", RagDollSkeletonEditor, editor::IObjectEditor)

RagDollSkeletonEditor::RagDollSkeletonEditor(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool RagDollSkeletonEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_instance = instance;
	m_asset = dynamic_type_cast< RagDollSkeletonAsset* >(object);
	if (!m_instance || !m_asset)
		return false;

	Ref< const PropertyGroup > settings = m_editor->getSettings();
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath");
	m_modelCachePath = settings->getProperty< std::wstring >(L"Pipeline.ModelCache.Path");

	Ref< ui::Splitter > splitter = new ui::Splitter();
	if (!splitter->create(parent, true, 300_ut))
		return false;

	// Left side; a horizontal splitter with the bone tree above the property view.
	Ref< ui::Splitter > leftSplitter = new ui::Splitter();
	if (!leftSplitter->create(splitter, false, 40_ut, true))
		return false;

	// Tree view over the rag doll bone hierarchy.
	m_treeView = new ui::TreeView();
	if (!m_treeView->create(leftSplitter, (ui::TreeView::WsDefault & ~ui::TreeView::WsAutoEdit) | ui::WsDoubleBuffer))
		return false;
	m_treeView->addEventHandler< ui::SelectionChangeEvent >(this, &RagDollSkeletonEditor::eventTreeSelect);
	m_treeView->addEventHandler< ui::MouseButtonDownEvent >(this, &RagDollSkeletonEditor::eventTreeButtonDown);

	// Property view over the selected bone (or the asset).
	m_propertyList = new ui::AutoPropertyList();
	m_propertyList->create(leftSplitter, ui::WsAccelerated | ui::WsTabStop | ui::AutoPropertyList::WsColumnHeader, this);
	m_propertyList->addEventHandler< ui::PropertyCommandEvent >(this, &RagDollSkeletonEditor::eventPropertyCommand);
	m_propertyList->addEventHandler< ui::PropertyContentChangeEvent >(this, &RagDollSkeletonEditor::eventPropertyChange);
	m_propertyList->setSeparator(160_ut);
	m_propertyList->setColumnName(0, i18n::Text(L"PROPERTY_COLUMN_NAME"));
	m_propertyList->setColumnName(1, i18n::Text(L"PROPERTY_COLUMN_VALUE"));
	m_propertyList->bind(m_asset);

	updateTreeView();

	// Preview side; a toolbar to select an animation skeleton and the 3D preview.
	Ref< ui::Container > containerPreview = new ui::Container();
	if (!containerPreview->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut)))
		return false;

	m_toolBar = new ui::ToolBar();
	if (!m_toolBar->create(containerPreview))
		return false;
	m_toolBar->addItem(new ui::ToolBarButton(L"Browse skeleton...", ui::Command(L"RagDoll.BrowseSkeleton"), ui::ToolBarButton::BsText));
	m_toolBar->addItem(new ui::ToolBarButton(L"Clear skeleton", ui::Command(L"RagDoll.ClearSkeleton"), ui::ToolBarButton::BsText));
	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &RagDollSkeletonEditor::eventToolBarClick);

	render::IRenderSystem* renderSystem = m_editor->getObjectStore()->get< render::IRenderSystem >();
	if (renderSystem)
	{
		m_previewControl = new RagDollPreviewControl();
		if (!m_previewControl->create(containerPreview, renderSystem, m_editor->getOutputDatabase()))
			return false;
		m_previewControl->setAsset(m_asset);
	}

	return true;
}

void RagDollSkeletonEditor::destroy()
{
	safeDestroy(m_previewControl);
	safeDestroy(m_propertyList);
	safeDestroy(m_treeView);
	m_toolBar = nullptr;
	m_instance = nullptr;
	m_asset = nullptr;
}

void RagDollSkeletonEditor::apply()
{
	m_propertyList->apply();
	m_instance->setObject(m_asset);
	if (m_previewControl)
		m_previewControl->update();
}

bool RagDollSkeletonEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.Copy")
		return m_propertyList->copy();
	else if (command == L"Editor.Paste")
	{
		if (m_propertyList->paste())
		{
			m_propertyList->apply();
			if (m_previewControl)
				m_previewControl->update();
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

void RagDollSkeletonEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

ui::Size RagDollSkeletonEditor::getPreferredSize() const
{
	return ui::Size(900, 600);
}

bool RagDollSkeletonEditor::resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const
{
	Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(guid);
	if (!instance)
		return false;

	resolved = instance->getPath();
	return true;
}

void RagDollSkeletonEditor::browseSkeleton()
{
	Ref< db::Instance > instance = m_editor->browseInstance(type_of< SkeletonAsset >());
	if (!instance)
		return;

	Ref< SkeletonAsset > skeletonAsset = instance->getObject< SkeletonAsset >();
	if (!skeletonAsset)
		return;

	const Path fileName = FileSystem::getInstance().getAbsolutePath(m_assetPath, skeletonAsset->getFileName());
	Ref< model::Model > model = model::ModelCache::getInstance().getMutable(m_modelCachePath, fileName, L"");
	if (!model)
		return;

	// Apply the asset scale so the overlay matches the built skeleton.
	model->apply(model::Transform(scale(skeletonAsset->getScale())));

	// Build a transient skeleton and evaluate object space joint transforms.
	Ref< Skeleton > skeleton = new Skeleton();
	for (const auto& modelJoint : model->getJoints())
	{
		Ref< Joint > joint = new Joint();
		if (modelJoint.getParent() != model::c_InvalidIndex)
			joint->setParent(modelJoint.getParent());
		joint->setName(modelJoint.getName());
		joint->setTransform(modelJoint.getTransform());
		skeleton->addJoint(joint);
	}

	if (m_previewControl)
		m_previewControl->setSkeleton(skeleton);
}

void RagDollSkeletonEditor::updateTreeView()
{
	m_treeView->removeAllItems();

	// Root item represents the asset itself, so asset level properties stay editable.
	Ref< ui::TreeViewItem > rootItem = m_treeView->createItem(nullptr, m_instance->getName(), 1);
	rootItem->setData(L"OBJECT", m_asset);

	if (m_asset->getRoot())
		buildBoneItems(rootItem, const_cast< RagDollBone* >(m_asset->getRoot()));

	rootItem->expand(true);
}

void RagDollSkeletonEditor::buildBoneItems(ui::TreeViewItem* parentItem, RagDollBone* bone)
{
	Ref< ui::TreeViewItem > item = m_treeView->createItem(parentItem, boneLabel(bone), 1);
	item->setData(L"OBJECT", bone);

	for (auto child : bone->getChildren())
		buildBoneItems(item, child);
}

void RagDollSkeletonEditor::selectTreeItem(const ISerializable* object)
{
	for (auto item : m_treeView->getItems(ui::TreeView::GfDescendants))
	{
		if (item->getData< ISerializable >(L"OBJECT") == object)
		{
			item->select();
			break;
		}
	}
}

void RagDollSkeletonEditor::eventPropertyCommand(ui::PropertyCommandEvent* event)
{
	const ui::Command& cmd = event->getCommand();
	if (cmd == L"Property.Add")
	{
		ui::ArrayPropertyItem* arrayItem = dynamic_type_cast< ui::ArrayPropertyItem* >(event->getItem());
		if (arrayItem)
		{
			if (arrayItem->getElementType())
			{
				const TypeInfo* objectType = m_editor->browseType(makeTypeInfoSet(*arrayItem->getElementType()), false, true);
				if (objectType)
				{
					Ref< ISerializable > object = dynamic_type_cast< ISerializable* >(objectType->createInstance());
					if (object)
					{
						m_propertyList->addObject(arrayItem, object);
						m_propertyList->apply();
						m_propertyList->refresh();
					}
				}
			}
			else
			{
				m_propertyList->apply();
				m_propertyList->refresh();
			}
		}
	}
	else if (cmd == L"Property.Remove")
	{
		ui::PropertyItem* removeItem = event->getItem();
		ui::PropertyItem* parentItem = removeItem->getParentItem();
		if (parentItem)
		{
			m_propertyList->removePropertyItem(parentItem, removeItem);
			m_propertyList->apply();
		}
	}
	else if (cmd == L"Property.Clear")
	{
		ui::BrowsePropertyItem* browseItem = dynamic_type_cast< ui::BrowsePropertyItem* >(event->getItem());
		if (browseItem)
		{
			browseItem->setValue(Guid());
			m_propertyList->apply();
		}
	}
	else if (cmd == L"Property.Browse")
	{
		ui::BrowsePropertyItem* browseItem = dynamic_type_cast< ui::BrowsePropertyItem* >(event->getItem());
		if (browseItem)
		{
			Ref< db::Instance > currentInstance = m_editor->getSourceDatabase()->getInstance(browseItem->getValue());
			Ref< db::Group > currentGroup = currentInstance ? currentInstance->getParent() : m_instance->getParent();

			Ref< db::Instance > instance;
			if (browseItem->getFilterType())
			{
				const TypeInfo* filterType = browseItem->getFilterType();
				T_ASSERT(filterType);
				instance = m_editor->browseInstance(*filterType, currentGroup);
			}
			else
				instance = m_editor->browseInstance(nullptr, currentGroup);

			if (instance)
			{
				browseItem->setValue(instance->getGuid());
				m_propertyList->apply();
			}
		}

		ui::ObjectPropertyItem* objectItem = dynamic_type_cast< ui::ObjectPropertyItem* >(event->getItem());
		if (objectItem)
		{
			const TypeInfo* objectType = objectItem->getObjectType();
			if (!objectType)
				objectType = &type_of< ISerializable >();

			if (!objectItem->getObject())
			{
				objectType = m_editor->browseType(makeTypeInfoSet(*objectType), false, true);
				if (objectType)
				{
					Ref< ISerializable > object = dynamic_type_cast< ISerializable* >(objectType->createInstance());
					if (object)
					{
						objectItem->setObject(object);
						m_propertyList->refresh(objectItem, object);
						m_propertyList->apply();
					}
				}
			}
			else
			{
				if (ui::ArrayPropertyItem* parentArrayItem = dynamic_type_cast< ui::ArrayPropertyItem* >(objectItem->getParentItem()))
					m_propertyList->removePropertyItem(parentArrayItem, objectItem);
				else
					objectItem->setObject(nullptr);

				m_propertyList->refresh(objectItem, nullptr);
				m_propertyList->apply();
			}
		}
	}
	else if (cmd == L"Property.Edit")
	{
		ui::BrowsePropertyItem* browseItem = dynamic_type_cast< ui::BrowsePropertyItem* >(event->getItem());
		if (browseItem)
		{
			const Guid instanceGuid = browseItem->getValue();
			if (instanceGuid.isNull() || !instanceGuid.isValid())
				return;

			Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(instanceGuid);
			if (instance)
				m_editor->openEditor(instance);
		}
	}

	m_propertyList->update();

	// Adding or removing bones changes the hierarchy; rebuild the tree.
	if (cmd == L"Property.Add" || cmd == L"Property.Remove")
		updateTreeView();

	if (m_previewControl)
		m_previewControl->update();
}

void RagDollSkeletonEditor::eventPropertyChange(ui::PropertyContentChangeEvent* event)
{
	m_propertyList->apply();
	if (m_previewControl)
		m_previewControl->update();
}

void RagDollSkeletonEditor::eventToolBarClick(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	if (command == L"RagDoll.BrowseSkeleton")
		browseSkeleton();
	else if (command == L"RagDoll.ClearSkeleton")
	{
		if (m_previewControl)
			m_previewControl->setSkeleton(nullptr);
	}
}

void RagDollSkeletonEditor::eventTreeSelect(ui::SelectionChangeEvent* event)
{
	const RefArray< ui::TreeViewItem > selected = m_treeView->getItems(ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);

	Ref< ISerializable > object;
	if (selected.size() == 1)
		object = selected[0]->getData< ISerializable >(L"OBJECT");
	if (!object)
		object = m_asset;

	// Show the selected bone (or the asset) in the property view.
	m_propertyList->bind(object);
	m_propertyList->update();

	// Highlight the selected bone in the 3D preview (null when the asset root is selected).
	if (m_previewControl)
		m_previewControl->setSelectedBone(selected.size() == 1 ? selected[0]->getData< RagDollBone >(L"OBJECT") : nullptr);
}

void RagDollSkeletonEditor::eventTreeButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	const RefArray< ui::TreeViewItem > selected = m_treeView->getItems(ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);
	if (selected.size() != 1)
		return;

	ui::TreeViewItem* item = selected[0];
	RagDollBone* bone = item->getData< RagDollBone >(L"OBJECT");

	// Build a context menu appropriate for the selected node.
	Ref< ui::Menu > menu = new ui::Menu();
	if (bone != nullptr)
	{
		menu->add(new ui::MenuItem(ui::Command(L"RagDoll.AddBone"), L"Add child bone"));
		menu->add(new ui::MenuItem(ui::Command(L"RagDoll.RemoveBone"), L"Remove bone"));
	}
	else if (m_asset->getRoot() == nullptr)
		menu->add(new ui::MenuItem(ui::Command(L"RagDoll.AddBone"), L"Add root bone"));
	else
		return;

	const ui::MenuItem* selectedMenuItem = menu->showModal(m_treeView, event->getPosition());
	if (selectedMenuItem == nullptr)
		return;

	const ui::Command& command = selectedMenuItem->getCommand();
	if (command == L"RagDoll.AddBone")
	{
		Ref< RagDollBone > newBone = new RagDollBone();
		if (bone != nullptr)
			bone->addChild(newBone);
		else
			m_asset->setRoot(newBone);

		updateTreeView();
		selectTreeItem(newBone);
	}
	else if (command == L"RagDoll.RemoveBone" && bone != nullptr)
	{
		ui::TreeViewItem* parentItem = item->getParent();
		RagDollBone* parentBone = parentItem != nullptr ? parentItem->getData< RagDollBone >(L"OBJECT") : nullptr;
		if (parentBone != nullptr)
			parentBone->removeChild(bone);
		else
			m_asset->setRoot(nullptr);

		updateTreeView();
		selectTreeItem(parentBone != nullptr ? (const ISerializable*)parentBone : (const ISerializable*)m_asset);
	}

	if (m_previewControl)
		m_previewControl->update();
}

}
