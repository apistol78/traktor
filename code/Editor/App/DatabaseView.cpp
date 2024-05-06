/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/Path.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Misc/WildCompare.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfmPrimitive.h"
#include "Core/Reflection/RfpMemberType.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Core/System/OS.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/Asset.h"
#include "Editor/Assets.h"
#include "Editor/IBrowsePreview.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPage.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/PipelineDependencySet.h"
#include "Editor/IWizardTool.h"
#include "Editor/App/BrowseTypeDialog.h"
#include "Editor/App/DatabaseView.h"
#include "Editor/App/InstanceClipboardData.h"
#include "Editor/App/NewInstanceDialog.h"
#include "Editor/PipelineDependency.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Clipboard.h"
#include "Ui/Edit.h"
#include "Ui/Menu.h"
#include "Ui/MenuItem.h"
#include "Ui/MessageBox.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/TableLayout.h"
#include "Ui/HierarchicalState.h"
#include "Ui/Splitter.h"
#include "Ui/PreviewList/PreviewItem.h"
#include "Ui/PreviewList/PreviewItems.h"
#include "Ui/PreviewList/PreviewList.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarDropDown.h"
#include "Ui/ToolBar/ToolBarEmbed.h"
#include "Ui/ToolBar/ToolBarSeparator.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewContentChangeEvent.h"
#include "Ui/TreeView/TreeViewDragEvent.h"
#include "Ui/TreeView/TreeViewItem.h"
#include "Ui/TreeView/TreeViewItemActivateEvent.h"
#include "Ui/TreeView/TreeViewItemStateChangeEvent.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.DatabaseView", DatabaseView, ui::Container)

	namespace
	{

class DefaultFilter : public DatabaseView::Filter
{
public:
	virtual bool acceptInstance(const db::Instance* instance) const override final
	{
		return is_type_of< ISerializable >(*instance->getPrimaryType());
	}

	virtual bool acceptEmptyGroups() const override final
	{
		return true;
	}
};

class TextFilter : public DatabaseView::Filter
{
public:
	explicit TextFilter(const std::wstring& filter)
	:	m_filter(filter)
	{
	}

	virtual bool acceptInstance(const db::Instance* instance) const override final
	{
		return m_filter.match(instance->getName());
	}

	virtual bool acceptEmptyGroups() const override final
	{
		return false;
	}

private:
	WildCompare m_filter;
};

class GuidFilter : public DatabaseView::Filter
{
public:
	explicit GuidFilter(const Guid& filter)
	:	m_filter(filter)
	{
	}

	virtual bool acceptInstance(const db::Instance* instance) const override final
	{
		return m_filter == instance->getGuid();
	}

	virtual bool acceptEmptyGroups() const override final
	{
		return false;
	}

private:
	Guid m_filter;
};

class TypeSetFilter : public DatabaseView::Filter
{
public:
	explicit TypeSetFilter(const TypeInfoSet& typeSet)
	:	m_typeSet(typeSet)
	{
	}

	virtual bool acceptInstance(const db::Instance* instance) const override final
	{
		const TypeInfo* instanceType = instance->getPrimaryType();
		if (!instanceType)
			return false;

		for (auto type : m_typeSet)
		{
			if (is_type_of(*type, *instanceType))
				return true;
		}

		return false;
	}

	virtual bool acceptEmptyGroups() const override final
	{
		return false;
	}

private:
	mutable TypeInfoSet m_typeSet;
};

class GuidSetFilter : public DatabaseView::Filter
{
public:
	explicit GuidSetFilter(const std::set< Guid >& guidSet)
	:	m_guidSet(guidSet)
	{
	}

	virtual bool acceptInstance(const db::Instance* instance) const override final
	{
		return m_guidSet.find(instance->getGuid()) != m_guidSet.end();
	}

	virtual bool acceptEmptyGroups() const override final
	{
		return false;
	}

private:
	mutable std::set< Guid > m_guidSet;
};

class CollectInstanceTypes
{
public:
	explicit CollectInstanceTypes(TypeInfoSet& outInstanceTypes)
	:	m_outInstanceTypes(outInstanceTypes)
	{
	}

	bool operator () (const db::Instance* instance) const
	{
		const TypeInfo* instanceType = instance->getPrimaryType();
		if (instanceType)
				m_outInstanceTypes.insert(instanceType);
		return false;
	}

private:
	TypeInfoSet& m_outInstanceTypes;
};

bool isInstanceInPrivate(const db::Instance* instance)
{
	db::Group* group = instance->getParent();
	while (group)
	{
		if (group->getName() == L"System")
			return true;
		group = group->getParent();
	}
	return false;
}

std::wstring getCategoryText(const TypeInfo* categoryType)
{
	const std::wstring id = replaceAll(toUpper(std::wstring(categoryType->getName())), L".", L"_");
	return i18n::Text(id, categoryType->getName());
}

std::wstring getUniqueInstanceName(const std::wstring& baseName, db::Group* group)
{
	if (!group->getInstance(baseName))
		return baseName;

	for (int32_t i = 2;; ++i)
	{
		const std::wstring sequenceName = baseName + L" (" + toString(i) + L")";
		if (!group->getInstance(sequenceName))
			return sequenceName;
	}

	return L"";
}

bool replaceIdentifiers(RfmCompound* reflection, const std::list< InstanceClipboardData::Instance >& instances)
{
	bool modified = false;

	for (uint32_t i = 0; i < reflection->getMemberCount(); ++i)
	{
		ReflectionMember* member = reflection->getMember(i);
		T_ASSERT(member);

		if (auto idMember = dynamic_type_cast< RfmPrimitiveGuid* >(member))
		{
			if (idMember->get().isNotNull())
			{
				for (const auto& instance : instances)
				{
					if (idMember->get() == instance.originalId)
					{
						idMember->set(instance.pasteId);
						modified = true;
					}
				}
			}
		}
		else if (auto objectMember = dynamic_type_cast< RfmObject* >(member))
		{
			Ref< Reflection > objectReflection = Reflection::create(objectMember->get());
			if (objectReflection)
			{
				if (replaceIdentifiers(objectReflection, instances))
				{
					objectReflection->apply(objectMember->get());
					modified = true;
				}
			}
		}
		else if (auto compoundMember = dynamic_type_cast< RfmCompound* >(member))
		{
			modified |= replaceIdentifiers(compoundMember, instances);
		}
	}

	return modified;
}

bool removeGroup(db::Group* group)
{
	// Recurse with child groups first.
	RefArray< db::Group > childGroups;
	group->getChildGroups(childGroups);
	for (auto childGroup : childGroups)
	{
		if (!removeGroup(childGroup))
			return false;
	}

	// No more child groups; now remove all child instances.
	RefArray< db::Instance > childInstances;
	group->getChildInstances(childInstances);
	for (auto childInstance : childInstances)
	{
		if (!childInstance->checkout())
			return false;
		if (!childInstance->remove())
			return false;
		if (!childInstance->commit())
			return false;
	}

	// Finally remove this group since it's now empty.
	if (!group->remove())
		return false;

	return true;
}

	}

DatabaseView::DatabaseView(IEditor* editor)
:	m_editor(editor)
,	m_filter(new DefaultFilter())
,	m_filterCountDown(-1)
{
}

bool DatabaseView::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut)))
		return false;

	m_toolSelection = new ui::ToolBar();
	if (!m_toolSelection->create(this))
		return false;
	m_toolSelection->addImage(new ui::StyleBitmap(L"Editor.Database.NameFilter"));
	m_toolSelection->addImage(new ui::StyleBitmap(L"Editor.Database.TypeFilter"));
	m_toolSelection->addImage(new ui::StyleBitmap(L"Editor.Database.ShowFiltered"));
	m_toolSelection->addImage(new ui::StyleBitmap(L"Editor.Database.Favorites"));

	m_toolFilterType = new ui::ToolBarButton(
		i18n::Text(L"DATABASE_FILTER"),
		0,
		ui::Command(L"Database.Filter"),
		ui::ToolBarButton::BsDefaultToggle
	);
	m_toolSelection->addItem(m_toolFilterType);

	m_toolFilterAssets = new ui::ToolBarButton(
		i18n::Text(L"DATABASE_FILTER_ASSETS"),
		1,
		ui::Command(L"Database.FilterAssets"),
		ui::ToolBarButton::BsDefaultToggle
	);
	m_toolSelection->addItem(m_toolFilterAssets);

	m_toolFilterShow = new ui::ToolBarButton(
		i18n::Text(L"DATABASE_FILTER_SHOW_FILTERED"),
		2,
		ui::Command(L"Database.ShowFiltered"),
		ui::ToolBarButton::BsDefaultToggle
	);
	m_toolSelection->addItem(m_toolFilterShow);

	m_toolFavoritesShow = new ui::ToolBarButton(
		i18n::Text(L"DATABASE_FILTER_SHOW_FAVORITES"),
		3,
		ui::Command(L"Database.ShowFavorites"),
		ui::ToolBarButton::BsDefaultToggle
	);
	m_toolSelection->addItem(m_toolFavoritesShow);

	m_toolSelection->addItem(new ui::ToolBarSeparator());

	m_editFilter = new ui::Edit();
	m_editFilter->create(m_toolSelection, L"", ui::WsNone);
	m_editFilter->addEventHandler< ui::KeyUpEvent >(this, &DatabaseView::eventFilterKey);
	m_toolSelection->addItem(new ui::ToolBarEmbed(m_editFilter, 80_ut));

	m_toolSelection->addItem(new ui::ToolBarSeparator());

	m_toolViewMode = new ui::ToolBarDropDown(ui::Command(L"Database.ViewModes"), 80_ut, i18n::Text(L"DATABASE_VIEW_MODE"));
	m_toolViewMode->add(i18n::Text(L"DATABASE_VIEW_MODE_HIERARCHY"));
	m_toolViewMode->add(i18n::Text(L"DATABASE_VIEW_MODE_SPLIT"));
	m_toolViewMode->select(
		m_editor->getSettings()->getProperty< int32_t >(L"Editor.DatabaseView", 1)
	);
	m_toolSelection->addItem(m_toolViewMode);

	m_toolSelection->addEventHandler< ui::ToolBarButtonClickEvent >(this, &DatabaseView::eventToolSelectionClicked);

	m_splitter = new ui::Splitter();
	m_splitter->create(this, false, 30_ut, true);

	m_treeDatabase = new ui::TreeView();
	if (!m_treeDatabase->create(m_splitter, (ui::TreeView::WsDefault | ui::TreeView::WsDrag | ui::WsAccelerated) & ~ui::WsClientBorder))
		return false;
	m_treeDatabase->addImage(new ui::StyleBitmap(L"Editor.Database.Folders", 0));
	m_treeDatabase->addImage(new ui::StyleBitmap(L"Editor.Database.Folders", 1));
	for (int32_t i = 0; i < 23; ++i)
		m_treeDatabase->addImage(new ui::StyleBitmap(L"Editor.Database.Types", i));
	for (int32_t i = 0; i < 23; ++i)
		m_treeDatabase->addImage(new ui::StyleBitmap(L"Editor.Database.TypesHidden", i));
	m_treeDatabase->addEventHandler< ui::TreeViewItemActivateEvent >(this, &DatabaseView::eventInstanceActivate);
	m_treeDatabase->addEventHandler< ui::SelectionChangeEvent >(this, &DatabaseView::eventInstanceSelect);
	m_treeDatabase->addEventHandler< ui::MouseButtonDownEvent >(this, &DatabaseView::eventInstanceButtonDown);
	m_treeDatabase->addEventHandler< ui::TreeViewContentChangeEvent >(this, &DatabaseView::eventInstanceRenamed);
	m_treeDatabase->addEventHandler< ui::DragEvent >(this, &DatabaseView::eventInstanceDrag);
	m_treeDatabase->setEnable(false);

	m_listInstances = new ui::PreviewList();
	if (!m_listInstances->create(m_splitter, ui::WsDoubleBuffer | ui::WsTabStop))
		return false;
	m_listInstances->addEventHandler< ui::MouseButtonDownEvent >(this, &DatabaseView::eventInstanceButtonDown);
	m_listInstances->addEventHandler< ui::MouseDoubleClickEvent >(this, &DatabaseView::eventInstancePreviewActivate);
	m_listInstances->addEventHandler< ui::DragEvent >(this, &DatabaseView::eventInstanceDrag);
	m_listInstances->setVisible(false);

	m_menuGroup[0] = new ui::Menu();
	m_menuGroup[1] = new ui::Menu();

	m_menuGroup[0]->add(new ui::MenuItem(ui::Command(L"Editor.Database.NewInstance"), i18n::Text(L"DATABASE_NEW_INSTANCE")));
	m_menuGroup[0]->add(new ui::MenuItem(ui::Command(L"Editor.Database.NewGroup"), i18n::Text(L"DATABASE_NEW_GROUP")));
	m_menuGroup[0]->add(new ui::MenuItem(ui::Command(L"Editor.Database.Rename"), i18n::Text(L"DATABASE_RENAME")));
	m_menuGroup[0]->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"DATABASE_DELETE")));
	m_menuGroup[0]->add(new ui::MenuItem(L"-"));
	m_menuGroup[0]->add(new ui::MenuItem(ui::Command(L"Editor.Database.FavoriteEntireGroup"), i18n::Text(L"DATABASE_FAVORITE_ENTIRE_GROUP")));
	m_menuGroup[0]->add(new ui::MenuItem(L"-"));
	m_menuGroup[0]->add(new ui::MenuItem(ui::Command(L"Editor.Paste"), i18n::Text(L"DATABASE_PASTE")));

	m_menuGroup[1]->add(new ui::MenuItem(ui::Command(L"Editor.Database.NewInstance"), i18n::Text(L"DATABASE_NEW_INSTANCE")));
	m_menuGroup[1]->add(new ui::MenuItem(ui::Command(L"Editor.Database.NewGroup"), i18n::Text(L"DATABASE_NEW_GROUP")));
	m_menuGroup[1]->add(new ui::MenuItem(ui::Command(L"Editor.Database.Rename"), i18n::Text(L"DATABASE_RENAME")));
	m_menuGroup[1]->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"DATABASE_DELETE")));
	m_menuGroup[1]->add(new ui::MenuItem(L"-"));
	m_menuGroup[1]->add(new ui::MenuItem(ui::Command(L"Editor.Database.UnFavoriteEntireGroup"), i18n::Text(L"DATABASE_UNFAVORITE_ENTIRE_GROUP")));
	m_menuGroup[1]->add(new ui::MenuItem(L"-"));
	m_menuGroup[1]->add(new ui::MenuItem(ui::Command(L"Editor.Paste"), i18n::Text(L"DATABASE_PASTE")));

	m_menuInstance = new ui::Menu();
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.ReplaceInstance"), i18n::Text(L"DATABASE_REPLACE_INSTANCE")));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.Rename"), i18n::Text(L"DATABASE_RENAME")));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"DATABASE_DELETE")));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.Clone"), i18n::Text(L"DATABASE_CLONE")));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.DefaultEditInstance"), i18n::Text(L"DATABASE_DEFAULT_EDIT_INSTANCE")));
	m_menuInstance->add(new ui::MenuItem(L"-"));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Copy"), i18n::Text(L"DATABASE_COPY")));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.CopyAll"), i18n::Text(L"DATABASE_COPY_ALL")));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.CopyInstanceId"), i18n::Text(L"DATABASE_COPY_INSTANCE_ID")));
	m_menuInstance->add(new ui::MenuItem(L"-"));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.OpenInNewEditor"), i18n::Text(L"DATABASE_OPEN_IN_NEW_EDITOR")));
	m_menuInstance->add(new ui::MenuItem(L"-"));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.FilterInstanceType"), i18n::Text(L"DATABASE_FILTER_TYPE")));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.FilterInstanceDepends"), i18n::Text(L"DATABASE_FILTER_DEPENDENCIES")));
	m_menuInstance->add(new ui::MenuItem(L"-"));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.ListInstanceDependents"), i18n::Text(L"DATABASE_LIST_DEPENDENTS")));
	m_menuInstance->add(new ui::MenuItem(L"-"));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.ToggleRoot"), i18n::Text(L"DATABASE_TOGGLE_AS_ROOT")));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.ToggleFavorite"), i18n::Text(L"DATABASE_TOGGLE_AS_FAVORITE")));
	m_menuInstance->add(new ui::MenuItem(L"-"));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.Build"), i18n::Text(L"DATABASE_BUILD")));
	m_menuInstance->add(new ui::MenuItem(ui::Command(L"Editor.Database.Rebuild"), i18n::Text(L"DATABASE_REBUILD")));

	m_menuInstanceAsset = new ui::Menu();
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.Edit"), i18n::Text(L"DATABASE_EDIT")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.Explore"), i18n::Text(L"DATABASE_EXPLORE")));
	m_menuInstanceAsset->add(new ui::MenuItem(L"-"));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.ReplaceInstance"), i18n::Text(L"DATABASE_REPLACE_INSTANCE")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.Rename"), i18n::Text(L"DATABASE_RENAME")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"DATABASE_DELETE")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.Clone"), i18n::Text(L"DATABASE_CLONE")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.DefaultEditInstance"), i18n::Text(L"DATABASE_DEFAULT_EDIT_INSTANCE")));
	m_menuInstanceAsset->add(new ui::MenuItem(L"-"));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Copy"), i18n::Text(L"DATABASE_COPY")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.CopyAll"), i18n::Text(L"DATABASE_COPY_ALL")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.CopyInstanceId"), i18n::Text(L"DATABASE_COPY_INSTANCE_ID")));
	m_menuInstanceAsset->add(new ui::MenuItem(L"-"));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.OpenInNewEditor"), i18n::Text(L"DATABASE_OPEN_IN_NEW_EDITOR")));
	m_menuInstanceAsset->add(new ui::MenuItem(L"-"));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.FilterInstanceType"), i18n::Text(L"DATABASE_FILTER_TYPE")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.FilterInstanceDepends"), i18n::Text(L"DATABASE_FILTER_DEPENDENCIES")));
	m_menuInstanceAsset->add(new ui::MenuItem(L"-"));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.ListInstanceDependents"), i18n::Text(L"DATABASE_LIST_DEPENDENTS")));
	m_menuInstanceAsset->add(new ui::MenuItem(L"-"));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.ToggleRoot"), i18n::Text(L"DATABASE_TOGGLE_AS_ROOT")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.ToggleFavorite"), i18n::Text(L"DATABASE_TOGGLE_AS_FAVORITE")));
	m_menuInstanceAsset->add(new ui::MenuItem(L"-"));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.Build"), i18n::Text(L"DATABASE_BUILD")));
	m_menuInstanceAsset->add(new ui::MenuItem(ui::Command(L"Editor.Database.Rebuild"), i18n::Text(L"DATABASE_REBUILD")));

	m_menuGroupWizards = new ui::MenuItem(i18n::Text(L"DATABASE_WIZARDS"));
	m_menuInstanceWizards = new ui::MenuItem(i18n::Text(L"DATABASE_WIZARDS"));
	m_menuGroup[0]->add(m_menuGroupWizards);
	m_menuGroup[1]->add(m_menuGroupWizards);
	m_menuInstance->add(m_menuInstanceWizards);
	m_menuInstanceAsset->add(m_menuInstanceWizards);

	m_iconsGroup = m_editor->getSettings()->getProperty< PropertyGroup >(L"Editor.Icons");
	if (!m_iconsGroup)
		return false;

	// Create browse preview generators.
	for (auto typeInfo : type_of< IBrowsePreview >().findAllOf(false))
	{
		Ref< const IBrowsePreview > browsePreview = checked_type_cast< const IBrowsePreview*, false >(typeInfo->createInstance());
		m_browsePreview.push_back(browsePreview);
	}

	addEventHandler< ui::SizeEvent >(this, &DatabaseView::eventSize);
	addEventHandler< ui::TimerEvent >(this, &DatabaseView::eventTimer);

	startTimer(100);

	setEnable(false);
	return true;
}

void DatabaseView::destroy()
{
	ui::Container::destroy();
}

void DatabaseView::setDatabase(db::Database* db)
{
	m_db = db;

	// Update wizards as this method is called after a workspace has been loaded.
	m_wizardTools.clear();

	const TypeInfoSet wizardToolTypes = type_of< IWizardTool >().findAllOf();
	if (!wizardToolTypes.empty())
	{
		// Create instances of all found wizards.
		for (const auto& wizardToolType : wizardToolTypes)
		{
			Ref< IWizardTool > wizard = dynamic_type_cast< IWizardTool* >(wizardToolType->createInstance());
			if (wizard)
				m_wizardTools.push_back(wizard);
		}

		// Sort wizard based on their description.
		m_wizardTools.sort([](const IWizardTool* a, const IWizardTool* b) {
			return compareIgnoreCase(a->getDescription(), b->getDescription()) < 0;
		});
	}

	// Ensure database views is cleaned.
	m_treeDatabase->removeAllItems();
	updateView();
}

void DatabaseView::updateView()
{
	const int32_t viewMode = m_toolViewMode->getSelected();
	Ref< ui::HierarchicalState > treeState = m_treeDatabase->captureState();

	m_treeDatabase->removeAllItems();
	m_listInstances->setItems(nullptr);
	m_listInstances->setVisible(false);

	if (m_db)
	{
		m_rootInstances.clear();
		m_favoriteInstances.clear();

		for (const auto& rootInstance : m_editor->getSettings()->getProperty< AlignedVector< std::wstring > >(L"Editor.RootInstances"))
			m_rootInstances.insert(Guid(rootInstance));

		for (const auto& favoriteInstance : m_editor->getSettings()->getProperty< AlignedVector< std::wstring > >(L"Editor.FavoriteInstances"))
			m_favoriteInstances.insert(Guid(favoriteInstance));

		if (viewMode == 0)	// Hierarchy
			buildTreeItemHierarchy(m_treeDatabase, 0, m_db->getRootGroup());
		else if (viewMode == 1)	// Split
		{
			m_listInstances->setVisible(true);
			buildTreeItemSplit(m_treeDatabase, 0, m_db->getRootGroup());
		}

		setEnable(true);
	}
	else
		setEnable(false);

	m_treeDatabase->applyState(treeState);
	if (viewMode == 2)
		updateGridInstances();

	m_splitter->update();
}

bool DatabaseView::highlight(const db::Instance* instance)
{
	RefArray< ui::TreeViewItem > items;
	m_treeDatabase->getItems(items, ui::TreeView::GfDescendants);
	for (auto item : items)
	{
		if (item->getData< db::Instance >(L"INSTANCE") == instance)
		{
			item->show();
			item->select();
			return true;
		}
	}
	return false;
}

bool DatabaseView::handleCommand(const ui::Command& command)
{
	ui::TreeViewItem* treeItem = nullptr;

	Ref< db::Group > group;
	Ref< db::Instance > instance;

	const int32_t viewMode = m_toolViewMode->getSelected();
	if (viewMode == 0)	// Hierarchy
	{	
		RefArray< ui::TreeViewItem > items;
		if (m_treeDatabase->getItems(items, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly) != 1)
			return false;

		treeItem = items.front();
		group = treeItem->getData< db::Group >(L"GROUP");
		instance = treeItem->getData< db::Instance >(L"INSTANCE");
	}
	else if (viewMode == 1)	// Split
	{
		RefArray< ui::TreeViewItem > items;
		if (m_treeDatabase->getItems(items, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly) != 1)
			return false;

		group = items.front()->getData< db::Group >(L"GROUP");

		auto selectedItem = m_listInstances->getSelectedItem();
		if (selectedItem)
			instance = selectedItem->getData< db::Instance >(L"INSTANCE");
	}

	if (group && instance)
	{
		if (command == L"Editor.Database.Edit")	// Edit
		{
			Ref< Asset > editAsset = instance->getObject< Asset >();
			if (editAsset)
			{
				const std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
				const Path filePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath) + editAsset->getFileName());
				OS::getInstance().editFile(filePath.getPathName());
			}
		}
		else if (command == L"Editor.Database.Explore")	// Explore
		{
			Ref< Asset > exploreAsset = instance->getObject< Asset >();
			if (exploreAsset)
			{
				const std::wstring assetPath = m_editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
				const Path filePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath) + exploreAsset->getFileName());
				OS::getInstance().exploreFile(filePath.getPathName());
			}
		}
		else if (command == L"Editor.Database.ReplaceInstance")	// Replace instance
		{
			const TypeInfoSet serializableTypeSet = makeTypeInfoSet< ISerializable >();
			BrowseTypeDialog browseTypeDlg(m_editor->checkoutGlobalSettings());
			browseTypeDlg.create(this, &serializableTypeSet, true, true);

			if (browseTypeDlg.showModal() == ui::DialogResult::Ok)
			{
				const TypeInfo* type = browseTypeDlg.getSelectedType();
				T_ASSERT(type);

				Ref< ISerializable > data = dynamic_type_cast< ISerializable* >(type->createInstance());
				T_ASSERT(data);

				if (instance->checkout())
				{
					instance->setObject(data);
					if (instance->commit())
						updateView();
					else
						log::error << L"Unable to commit instance." << Endl;
				}
				else
					log::error << L"Unable to checkout instance." << Endl;
			}

			browseTypeDlg.destroy();
		}
		else if (command == L"Editor.Database.Rename")	// Rename
		{
			if (treeItem)
				treeItem->edit();
		}
		else if (command == L"Editor.Delete")	// Delete
		{
			if (ui::MessageBox::show(this, i18n::Text(L"DATABASE_DELETE_ARE_YOU_SURE"), i18n::Text(L"DATABASE_DELETE_INSTANCE"), ui::MbYesNo | ui::MbIconQuestion) != ui::DialogResult::Yes)
				return false;

			if (!instance->checkout())
				return false;
			if (!instance->remove())
				return false;
			if (!instance->commit())
				return false;

			updateView();
		}
		else if (command == L"Editor.Database.Clone")	// Clone
		{
			Ref< ISerializable > object = instance->getObject< ISerializable >();
			if (!object)
			{
				log::error << L"Unable to checkout instance." << Endl;
				return false;
			}

			object = m_editor->cloneAsset(object);
			if (!object)
			{
				log::error << L"Unable to create clone." << Endl;
				return false;
			}

			Ref< db::Instance > instanceClone = group->createInstance(instance->getName() + L" (clone)");
			if (!instanceClone)
			{
				log::error << L"Unable to create clone instance." << Endl;
				return false;
			}

			instanceClone->setObject(object);

			if (!instanceClone->commit())
			{
				log::error << L"Unable to commit clone instance." << Endl;
				return false;
			}

			updateView();
		}
		else if (command == L"Editor.Database.DefaultEditInstance")	// Default edit instance
		{
			m_editor->openDefaultEditor(instance);
		}
		else if (command == L"Editor.Copy")		// Copy instance
		{
			Ref< ISerializable > object = instance->getObject< ISerializable >();
			if (!object)
			{
				log::error << L"Unable to read instance object." << Endl;
				return false;
			}

			object = m_editor->cloneAsset(object);
			if (!object)
			{
				log::error << L"Unable to create clone." << Endl;
				return false;
			}

			Ref< InstanceClipboardData > instanceClipboardData = new InstanceClipboardData();
			instanceClipboardData->addInstance(instance->getName(), object);

			ui::Application::getInstance()->getClipboard()->setObject(instanceClipboardData);
		}
		else if (command == L"Editor.CopyAll")	// Copy instance, including all dependencies.
		{
			Ref< ISerializable > object = instance->getObject< ISerializable >();
			if (!object)
			{
				log::error << L"Unable to read instance object." << Endl;
				return false;
			}

			object = m_editor->cloneAsset(object);
			if (!object)
			{
				log::error << L"Unable to create clone." << Endl;
				return false;
			}

			PipelineDependencySet dependencySet;
			Ref< IPipelineDepends > depends = m_editor->createPipelineDepends(&dependencySet, std::numeric_limits< uint32_t >::max());
			if (!depends)
				return false;

			depends->addDependency(object);
			depends->waitUntilFinished();

			Ref< InstanceClipboardData > instanceClipboardData = new InstanceClipboardData();
			instanceClipboardData->addInstance(instance->getName(), object);

			const bool rootIsPrivate = isInstanceInPrivate(instance);

			for (uint32_t i = 0; i < dependencySet.size(); ++i)
			{
				const PipelineDependency* dependency = dependencySet.get(i);
				T_ASSERT(dependency);

				Ref< db::Instance > dependentInstance = m_db->getInstance(dependency->outputGuid);
				if (dependentInstance && (rootIsPrivate || !isInstanceInPrivate(dependentInstance)))
				{
					Ref< ISerializable > dependentObject = dependentInstance->getObject();
					if (!dependentObject)
					{
						log::error << L"Unable to read instance object." << Endl;
						return false;
					}

					dependentObject = m_editor->cloneAsset(dependentObject);
					if (!dependentObject)
					{
						log::error << L"Unable to create clone." << Endl;
						return false;
					}

					instanceClipboardData->addInstance(
						dependentInstance->getName(),
						dependentObject,
						dependentInstance->getGuid()
					);
				}
			}

			ui::Application::getInstance()->getClipboard()->setObject(instanceClipboardData);
		}
		else if (command == L"Editor.CopyInstanceId")	// Copy instance ID.
		{
			const Guid id = instance->getGuid();
			ui::Application::getInstance()->getClipboard()->setText(id.format());
		}
		else if (command == L"Editor.OpenInNewEditor")	// Open in new editor.
		{
			m_editor->openInNewEditor(instance);
		}
		else if (command == L"Editor.Database.FilterInstanceType")	// Filter on type.
		{
			filterType(instance);
		}
		else if (command == L"Editor.Database.FilterInstanceDepends")	// Filter on dependencies
		{
			filterDependencies(instance);
		}
		else if (command == L"Editor.Database.ListInstanceDependents")
		{
			listInstanceDependents(instance);
		}
		else if (command == L"Editor.Database.ToggleRoot")	// Toggle root flag.
		{
			const Guid instanceGuid = instance->getGuid();

			auto it = m_rootInstances.find(instanceGuid);
			if (it == m_rootInstances.end())
				m_rootInstances.insert(instanceGuid);
			else
				m_rootInstances.erase(it);

			AlignedVector< std::wstring > rootInstances;
			for (const auto& rootInstance : m_rootInstances)
				rootInstances.push_back(rootInstance.format());

			Ref< PropertyGroup > workspaceSettings = m_editor->checkoutWorkspaceSettings();
			workspaceSettings->setProperty< PropertyStringArray >(L"Editor.RootInstances", rootInstances);
			m_editor->commitWorkspaceSettings();

			updateView();
		}
		else if (command == L"Editor.Database.ToggleFavorite")	// Toggle favorite flag.
		{
			const Guid instanceGuid = instance->getGuid();

			auto it = m_favoriteInstances.find(instanceGuid);
			if (it == m_favoriteInstances.end())
				m_favoriteInstances.insert(instanceGuid);
			else
				m_favoriteInstances.erase(it);

			AlignedVector< std::wstring > favoriteInstances;
			for (const auto& favoriteInstance : m_favoriteInstances)
				favoriteInstances.push_back(favoriteInstance.format());

			Ref< PropertyGroup > globalSettings = m_editor->checkoutGlobalSettings();
			globalSettings->setProperty< PropertyStringArray >(L"Editor.FavoriteInstances", favoriteInstances);
			m_editor->commitGlobalSettings();

			updateView();
		}
		else if (command == L"Editor.Database.Build")	// Build asset
		{
			m_editor->buildAsset(instance->getGuid(), false);
		}
		else if (command == L"Editor.Database.Rebuild")	// Rebuild asset
		{
			m_editor->buildAsset(instance->getGuid(), true);
		}
		else if (command == L"Editor.Database.Wizard")
		{
			Ref< IWizardTool > wizard = m_wizardTools[command.getId()];
			if (wizard->launch(this, m_editor, group, instance))
				updateView();
		}
		else
			return false;
	}
	else if (group)
	{
		if (command == L"Editor.Database.NewInstance")	// New instance...
		{
			// Count number of individual type groups to determine initial folder selected.
			std::map< std::wstring, int32_t > groupNames;

			RefArray< db::Instance > childInstances;
			group->getChildInstances(childInstances);

			for (auto childInstance : childInstances)
			{
				const TypeInfo* type = childInstance->getPrimaryType();
				if (type)
				{
					const std::wstring typeName = type->getName();
					const size_t ln = typeName.find_last_of(L'.');
					if (ln > 0 && ln != typeName.npos)
					{
						const std::wstring groupName = typeName.substr(0, ln);
						groupNames[groupName]++;
					}
				}
			}

			std::vector< std::pair< std::wstring, int32_t > > tmp;
			tmp.insert(tmp.begin(), groupNames.begin(), groupNames.end());

			std::sort(tmp.begin(), tmp.end(), [](
				const std::pair< std::wstring, int32_t >& lh,
				const std::pair< std::wstring, int32_t >& rh
			) {
				return lh.second > rh.second;
			});

			const std::wstring initialGroupHint = !tmp.empty() ? tmp.front().first : L"";

			NewInstanceDialog newInstanceDlg(m_editor->checkoutGlobalSettings());
			newInstanceDlg.create(this, initialGroupHint);

			if (newInstanceDlg.showModal() == ui::DialogResult::Ok && newInstanceDlg.getType() != nullptr)
			{
				const TypeInfo* type = newInstanceDlg.getType();

				const std::wstring instanceName = newInstanceDlg.getInstanceName();
				T_ASSERT(!instanceName.empty());

				Ref< ISerializable > data = dynamic_type_cast< ISerializable* >(type->createInstance());
				T_ASSERT(data);

				Ref< db::Instance > instance = group->createInstance(instanceName);
				if (instance)
				{
					instance->setObject(data);
					if (instance->commit())
						updateView();
				}
			}

			newInstanceDlg.destroy();
		}
		else if (command == L"Editor.Database.NewGroup")	// New group...
		{
			Ref< db::Group > newGroup = group->createGroup(i18n::Text(L"DATABASE_NEW_GROUP_UNNAMED"));
			if (newGroup)
				updateView();
		}
		else if (command == L"Editor.Database.Rename")	// Rename
		{
			if (treeItem)
				treeItem->edit();
		}
		else if (command == L"Editor.Delete")	// Delete
		{
			if (ui::MessageBox::show(this, i18n::Text(L"DATABASE_DELETE_ARE_YOU_SURE"), i18n::Text(L"DATABASE_DELETE_GROUP"), ui::MbYesNo | ui::MbIconQuestion) != ui::DialogResult::Yes)
				return false;

			if (!removeGroup(group))
				return false;

			updateView();
		}
		else if (command == L"Editor.Paste")	// Paste instance into group
		{
			Ref< InstanceClipboardData > instanceClipboardData = dynamic_type_cast< InstanceClipboardData* >(
				ui::Application::getInstance()->getClipboard()->getObject()
			);
			if (!instanceClipboardData)
				return false;

			std::list< InstanceClipboardData::Instance > pasteInstances = instanceClipboardData->getInstances();

			// Create unique identifiers for each pasted instance.
			for (auto& pasteInstance : pasteInstances)
				pasteInstance.pasteId = Guid::create();

			// Replace all occurrences of original identifiers with new identifiers.
			for (auto& pasteInstance : pasteInstances)
			{
				Ref< Reflection > reflection = Reflection::create(pasteInstance.object);
				if (!reflection)
					return false;

				if (replaceIdentifiers(reflection, pasteInstances))
					reflection->apply(pasteInstance.object);
			}

			for (const auto& pasteInstance : pasteInstances)
			{
				const std::wstring pasteName = getUniqueInstanceName(pasteInstance.name, group);

				Ref< db::Instance > instanceCopy = group->createInstance(pasteName, db::CifDefault, &pasteInstance.pasteId);
				if (!instanceCopy)
				{
					log::error << L"Unable to create instance copy." << Endl;
					return false;
				}

				instanceCopy->setObject(pasteInstance.object);

				if (!instanceCopy->commit())
				{
					log::error << L"Unable to commit instance copy." << Endl;
					return false;
				}

				updateView();
			}
		}
		else if (command == L"Editor.Database.FavoriteEntireGroup" || command == L"Editor.Database.UnFavoriteEntireGroup")
		{
			const bool addToFavorites = bool(command == L"Editor.Database.FavoriteEntireGroup");

			RefArray< db::Instance > instances;
			db::recursiveFindChildInstances(group, db::FindInstanceAll(), instances);
			for (auto instance : instances)
			{
				Guid instanceGuid = instance->getGuid();
				if (addToFavorites)
					m_favoriteInstances.insert(instanceGuid);
				else
				{
					auto it = m_favoriteInstances.find(instanceGuid);
					if (it != m_favoriteInstances.end())
						m_favoriteInstances.erase(it);
				}
			}

			AlignedVector< std::wstring > favoriteInstances;
			for (const auto& favoriteInstance : m_favoriteInstances)
				favoriteInstances.push_back(favoriteInstance.format());

			Ref< PropertyGroup > globalSettings = m_editor->checkoutGlobalSettings();
			globalSettings->setProperty< PropertyStringArray >(L"Editor.FavoriteInstances", favoriteInstances);
			m_editor->commitGlobalSettings();

			updateView();
		}
		else if (command == L"Editor.Database.Wizard")
		{
			Ref< IWizardTool > wizard = m_wizardTools[command.getId()];
			if (wizard->launch(this, m_editor, group, 0))
				updateView();
		}
		else
			return false;
	}

	return true;
}

void DatabaseView::setEnable(bool enable)
{
	m_toolSelection->setEnable(enable);
	m_toolSelection->update();

	m_treeDatabase->setEnable(enable);
	m_treeDatabase->update();

	ui::Container::setEnable(enable);
}

int32_t DatabaseView::getIconIndex(const TypeInfo* instanceType) const
{
	int32_t iconIndex = 2;

	const auto& icons = m_iconsGroup->getValues();
	for (auto i = icons.begin(); i != icons.end(); ++i)
	{
		const TypeInfo* iconType = TypeInfo::find(i->first.c_str());
		if (iconType && is_type_of(*iconType, *instanceType))
		{
			iconIndex = PropertyInteger::get(i->second);
			break;
		}
	}

	return iconIndex;
}

Ref< ui::TreeViewItem > DatabaseView::buildTreeItemHierarchy(ui::TreeView* treeView, ui::TreeViewItem* parentItem, db::Group* group)
{
	Ref< ui::TreeViewItem > groupItem = treeView->createItem(parentItem, group->getName(), 1);
	groupItem->setImage(0, 0, 1);
	groupItem->setData(L"GROUP", group);
	
	// Highlight linked groups and ensure they cannot be renamed.
	if ((group->getFlags() & db::GfLink) != 0)
	{
		groupItem->setEditable(false);
		groupItem->setTextColor(getStyleSheet()->getColor(this, L"color-link"));
		groupItem->setBold(true);
	}
	else
		groupItem->setEditable(true);

	// Expand root groups by default.
	if (!parentItem)
		groupItem->expand();

	RefArray< db::Group > childGroups;
	group->getChildGroups(childGroups);
	childGroups.sort([](const db::Group* a, const db::Group* b) {
		return compareIgnoreCase(a->getName(), b->getName()) < 0;
	});

	for (auto childGroup : childGroups)
		buildTreeItemHierarchy(treeView, groupItem, childGroup);

	const bool showFiltered = m_toolFilterShow->isToggled();
	const bool showFavorites = m_toolFavoritesShow->isToggled();

	RefArray< db::Instance > childInstances;
	group->getChildInstances(childInstances);
	childInstances.sort([](const db::Instance* a, const db::Instance* b) {
		return compareIgnoreCase(a->getName(), b->getName()) < 0;
	});

	for (auto childInstance : childInstances)
	{
		const TypeInfo* primaryType = childInstance->getPrimaryType();
		if (!primaryType)
			continue;

		if (showFavorites)
		{
			if (m_favoriteInstances.find(childInstance->getGuid()) == m_favoriteInstances.end())
				continue;
		}

		int32_t iconIndex = getIconIndex(primaryType);
		if (!showFiltered)
		{
			if (!m_filter->acceptInstance(childInstance))
				continue;
		}
		else
		{
			if (!m_filter->acceptInstance(childInstance))
				iconIndex += 23;
		}

		Ref< ui::TreeViewItem > instanceItem = treeView->createItem(groupItem, childInstance->getName(), 1);
		instanceItem->setImage(0, iconIndex);

		if (m_rootInstances.find(childInstance->getGuid()) != m_rootInstances.end())
			instanceItem->setBold(true);

		instanceItem->setEditable(true);
		instanceItem->setData(L"GROUP", group);
		instanceItem->setData(L"INSTANCE", childInstance);
	}

	// Remove group if it's empty.
	if ((showFavorites || !m_filter->acceptEmptyGroups()) && !groupItem->hasChildren())
	{
		treeView->removeItem(groupItem);
		groupItem = nullptr;
	}

	return groupItem;
}

Ref< ui::TreeViewItem > DatabaseView::buildTreeItemSplit(ui::TreeView* treeView, ui::TreeViewItem* parentItem, db::Group* group)
{
	Ref< ui::TreeViewItem > groupItem = treeView->createItem(parentItem, group->getName(), 1);
	groupItem->setImage(0, 0, 1);
	groupItem->setData(L"GROUP", group);

	// Highlight linked groups and ensure they cannot be renamed.
	if ((group->getFlags() & db::GfLink) != 0)
	{
		groupItem->setEditable(false);
		groupItem->setTextColor(ui::ColorReference(this, L"color-link"));
		groupItem->setBold(true);
	}
	else
		groupItem->setEditable(true);

	RefArray< db::Group > childGroups;
	group->getChildGroups(childGroups);
	childGroups.sort([](const db::Group* a, const db::Group* b) {
		return compareIgnoreCase(a->getName(), b->getName()) < 0;
	});

	for (auto childGroup : childGroups)
		buildTreeItemSplit(treeView, groupItem, childGroup);

	return groupItem;
}

void DatabaseView::updateGridInstances()
{
	// Grid is only visible in "split" mode.
	const int32_t viewMode = m_toolViewMode->getSelected();
	if (viewMode != 1)
		return;

	m_listInstances->setItems(nullptr);

	RefArray< ui::TreeViewItem > items;
	if (m_treeDatabase->getItems(items, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly) <= 0)
		return;

	RefArray< db::Instance > childInstances;
	for (auto item : items)
	{
		db::Group* group = item->getData< db::Group >(L"GROUP");
		if (group)
			db::findChildInstances(group, db::FindInstanceAll(), childInstances);
	}
	childInstances.sort([](const db::Instance* a, const db::Instance* b) {
		return compareIgnoreCase(a->getName(), b->getName()) < 0;
	});

	const bool showFiltered = m_toolFilterShow->isToggled();
	const bool showFavorites = m_toolFavoritesShow->isToggled();

	Ref< ui::PreviewItems > previewItems = new ui::PreviewItems();

	for (auto childInstance : childInstances)
	{
		const TypeInfo* primaryType = childInstance->getPrimaryType();
		if (!primaryType)
			continue;

		if (showFavorites)
		{
			if (m_favoriteInstances.find(childInstance->getGuid()) == m_favoriteInstances.end())
				continue;
		}

		int32_t iconIndex = getIconIndex(primaryType);
		if (!showFiltered)
		{
			if (!m_filter->acceptInstance(childInstance))
				continue;
		}
		else
		{
			if (!m_filter->acceptInstance(childInstance))
				iconIndex += 23;
		}

		Ref< ui::PreviewItem > item = new ui::PreviewItem();
		item->setText(childInstance->getName());
		item->setSubText(getCategoryText(primaryType));
		item->setData(L"GROUP", childInstance->getParent());
		item->setData(L"INSTANCE", childInstance);
		previewItems->add(item);

		// Get thumbnail preview of instance.
		const TypeInfo* instanceType = childInstance->getPrimaryType();
		for (auto browsePreview : m_browsePreview)
		{
			TypeInfoSet previewTypes = browsePreview->getPreviewTypes();
			if (previewTypes.find(instanceType) != previewTypes.end())
			{
				item->setImage(browsePreview->generate(
					m_editor,
					childInstance
				));
				break;
			}
		}
	}

	m_listInstances->setItems(previewItems);
}

void DatabaseView::filterType(db::Instance* instance)
{
	TypeInfoSet typeSet;
	typeSet.insert(instance->getPrimaryType());
	m_editFilter->setText(L"");
	m_filter.reset(new TypeSetFilter(typeSet));
	m_toolFilterType->setToggled(true);
	m_toolFilterAssets->setToggled(false);
	updateView();
}

void DatabaseView::filterDependencies(db::Instance* instance)
{
	if (!instance)
		return;

	PipelineDependencySet dependencySet;
	Ref< IPipelineDepends > depends = m_editor->createPipelineDepends(&dependencySet, std::numeric_limits< uint32_t >::max());
	if (!depends)
		return;

	depends->addDependency(instance->getObject());
	depends->waitUntilFinished();

	// Create set of all dependency guids, include root guid as well.
	std::set< Guid > guidSet;
	guidSet.insert(instance->getGuid());

	for (uint32_t i = 0; i < dependencySet.size(); ++i)
	{
		const PipelineDependency* dependency = dependencySet.get(i);
		T_ASSERT(dependency);

		if (dependency->outputGuid.isNotNull())
			guidSet.insert(dependency->outputGuid);
	}

	m_editFilter->setText(L"");
	m_filter.reset(new GuidSetFilter(guidSet));
	m_toolFilterType->setToggled(true);
	m_toolFilterAssets->setToggled(false);

	updateView();
}

void DatabaseView::listInstanceDependents(db::Instance* instance)
{
	if (!instance)
		return;

	const Guid findInstanceGuid = instance->getGuid();

	for (const auto& rootGuid : m_rootInstances)
	{
		Ref< db::Instance > rootInstance = m_db->getInstance(rootGuid);
		if (!rootInstance)
			continue;

		PipelineDependencySet dependencySet;
		Ref< IPipelineDepends > depends = m_editor->createPipelineDepends(&dependencySet, std::numeric_limits< uint32_t >::max());
		if (!depends)
			return;

		depends->addDependency(rootInstance->getObject());
		depends->waitUntilFinished();

		for (uint32_t j = 0; j < dependencySet.size(); ++j)
		{
			const PipelineDependency* dependency = dependencySet.get(j);
			T_ASSERT(dependency != nullptr);

			if (dependency->sourceInstanceGuid == findInstanceGuid)
			{
				for (uint32_t k = 0; k < dependencySet.size(); ++k)
				{
					const PipelineDependency* parentDependency = dependencySet.get(k);
					T_ASSERT(parentDependency != nullptr);

					if (parentDependency->children.find(j) != parentDependency->children.end())
						log::info << parentDependency->sourceInstanceGuid.format() << Endl;
				}
			}
		}
	}
}

void DatabaseView::eventToolSelectionClicked(ui::ToolBarButtonClickEvent* event)
{
	const ui::Command& cmd = event->getCommand();
	if (cmd == L"Database.Filter")
	{
		if (m_toolFilterType->isToggled())
		{
			const TypeInfo* filterType = m_editor->browseType(makeTypeInfoSet< ISerializable >(), false, false);
			if (filterType)
			{
				TypeInfoSet typeSet;
				typeSet.insert(filterType);
				m_editFilter->setText(L"");
				m_filter.reset(new TypeSetFilter(typeSet));
				m_toolFilterAssets->setToggled(false);
			}
			else
				m_toolFilterType->setToggled(false);
		}
		if (!m_toolFilterType->isToggled())
			m_filter.reset(new DefaultFilter());
	}
	else if (cmd == L"Database.FilterAssets")
	{
		if (m_toolFilterAssets->isToggled())
		{
			RefArray< db::Instance > assetsInstances;
			db::recursiveFindChildInstances(
				m_db->getRootGroup(),
				db::FindInstanceByType(type_of< Assets >()),
				assetsInstances
			);

			std::set< Guid > guidSet;
			for (auto assetsInstance : assetsInstances)
			{
				guidSet.insert(assetsInstance->getGuid());

				PipelineDependencySet dependencySet;
				Ref< IPipelineDepends > depends = m_editor->createPipelineDepends(&dependencySet, std::numeric_limits< uint32_t >::max());
				if (!depends)
					return;

				depends->addDependency(assetsInstance->getObject());
				depends->waitUntilFinished();

				for (uint32_t j = 0; j < dependencySet.size(); ++j)
				{
					const PipelineDependency* dependency = dependencySet.get(j);
					T_ASSERT(dependency);

					if (dependency->outputGuid.isNotNull())
						guidSet.insert(dependency->outputGuid);
				}
			}

			m_editFilter->setText(L"");
			m_filter.reset(new GuidSetFilter(guidSet));
			m_toolFilterType->setToggled(false);
		}
		if (!m_toolFilterAssets->isToggled())
			m_filter.reset(new DefaultFilter());
	}
	else if (cmd == L"Database.ViewModes")
	{
		const int32_t viewMode = m_toolViewMode->getSelected();
		m_editor->checkoutGlobalSettings()->setProperty< PropertyInteger >(L"Editor.DatabaseView", viewMode);
	}

	updateView();
}

void DatabaseView::eventFilterKey(ui::KeyUpEvent* event)
{
	m_filterText = m_editFilter->getText();
	m_filterCountDown = 5;
}

void DatabaseView::eventSize(ui::SizeEvent* event)
{
	const ui::Size& sz = event->getSize();
	if (sz.cx >= sz.cy)
		m_splitter->setVertical(true);
	else
		m_splitter->setVertical(false);
}

void DatabaseView::eventTimer(ui::TimerEvent* event)
{
	if (--m_filterCountDown == 0)
	{
		if (!m_filterText.empty())
		{
			const Guid filterGuid(m_filterText);
			if (filterGuid.isValid() && filterGuid.isNotNull())
				m_filter.reset(new GuidFilter(filterGuid));
			else
				m_filter.reset(new TextFilter(m_filterText));
		}
		else
			m_filter.reset(new DefaultFilter());

		m_toolFilterType->setToggled(false);
		m_toolFilterAssets->setToggled(false);

		updateView();
	}
}

void DatabaseView::eventInstanceActivate(ui::TreeViewItemActivateEvent* event)
{
	Ref< ui::TreeViewItem > item = event->getItem();

	Ref< db::Instance > instance = item->getData< db::Instance >(L"INSTANCE");
	if (!instance)
		return;

	m_editor->openEditor(instance);
	event->consume();
}

void DatabaseView::eventInstanceSelect(ui::SelectionChangeEvent* event)
{
	RefArray< ui::TreeViewItem > items;
	if (m_treeDatabase->getItems(items, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly) <= 0)
		return;

	updateGridInstances();
}

void DatabaseView::eventInstanceButtonDown(ui::MouseButtonDownEvent* event)
{
	if (event->getButton() != ui::MbtRight)
		return;

	ui::Associative* selectedItem = nullptr;
	if (event->getSender() == m_treeDatabase)
	{
		RefArray< ui::TreeViewItem > items;
		if (m_treeDatabase->getItems(items, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly) != 1)
			return;
		selectedItem = items.front();
	}
	else if (event->getSender() == m_listInstances)
	{
		if ((selectedItem = m_listInstances->getSelectedItem()) == nullptr)
			return;
	}
	else
		return;

	T_FATAL_ASSERT(selectedItem);

	Ref< db::Group > group = selectedItem->getData< db::Group >(L"GROUP");
	Ref< db::Instance > instance = selectedItem->getData< db::Instance >(L"INSTANCE");

	if (group && instance)
	{
		m_menuInstanceWizards->removeAll();

		// Rebuild instance wizard menu to ensure only valid wizards are enabled.
		int32_t nextWizardId = 0;
		for (auto wizardTool : m_wizardTools)
		{
			const std::wstring wizardDescription = wizardTool->getDescription();
			T_ASSERT(!wizardDescription.empty());

			const int32_t wizardId = nextWizardId++;

			if ((wizardTool->getFlags() & IWizardTool::WfInstance) != 0)
			{
				Ref< ui::MenuItem > item = new ui::MenuItem(ui::Command(wizardId, L"Editor.Database.Wizard"), wizardDescription);
				const TypeInfoSet supportedTypes = wizardTool->getSupportedTypes();
				if (!supportedTypes.empty())
				{
					bool enabled = false;
					for (auto supportedType : supportedTypes)
					{
						if (is_type_of(*supportedType, *instance->getPrimaryType()))
						{
							enabled = true;
							break;
						}
					}
					item->setEnable(enabled);
				}
				m_menuInstanceWizards->add(item);
			}
		}

		Ref< ui::Menu > menuInstance;
		if (is_type_of< Asset >(*instance->getPrimaryType()))
			menuInstance = m_menuInstanceAsset;
		else
			menuInstance = m_menuInstance;

		const ui::MenuItem* selected = menuInstance->showModal(
			mandatory_non_null_type_cast< ui::Widget* >(event->getSender()),
			event->getPosition()
		);
		if (selected)
			handleCommand(selected->getCommand());
	}
	else if (group)
	{
		m_menuGroupWizards->removeAll();

		// Rebuild instance wizard menu to ensure only valid wizards are enabled.
		int32_t nextWizardId = 0;
		for (auto wizardTool : m_wizardTools)
		{
			const std::wstring wizardDescription = wizardTool->getDescription();
			T_ASSERT(!wizardDescription.empty());

			const int32_t wizardId = nextWizardId++;

			if ((wizardTool->getFlags() & IWizardTool::WfGroup) != 0)
			{
				Ref< ui::MenuItem > item = new ui::MenuItem(ui::Command(wizardId, L"Editor.Database.Wizard"), wizardDescription);
				m_menuGroupWizards->add(item);
			}
		}

		const bool showFavorites = m_toolFavoritesShow->isToggled();
		const ui::MenuItem* selected = m_menuGroup[showFavorites ? 1 : 0]->showModal(
			mandatory_non_null_type_cast< ui::Widget* >(event->getSender()),
			event->getPosition()
		);
		if (selected)
			handleCommand(selected->getCommand());
	}

	event->consume();
}

void DatabaseView::eventInstanceRenamed(ui::TreeViewContentChangeEvent* event)
{
	Ref< ui::TreeViewItem > treeItem = event->getItem();
	if (!treeItem)
		return;

	Ref< db::Instance > instance = treeItem->getData< db::Instance >(L"INSTANCE");
	Ref< db::Group > group = treeItem->getData< db::Group >(L"GROUP");

	bool result = false;

	if (instance && group)
	{
		if (instance->checkout())
		{
			result = instance->setName(treeItem->getText());
			result &= instance->commit();
		}
	}
	else if (group)
		result = group->rename(treeItem->getText());

	if (result)
		event->consume();
}

void DatabaseView::eventInstanceDrag(ui::DragEvent* event)
{
	Ref< db::Instance > instance;

	if (auto treeViewDrag = dynamic_type_cast<ui::TreeViewDragEvent*>(event))
		instance = treeViewDrag->getItem()->getData< db::Instance >(L"INSTANCE");
	else
		instance = m_listInstances->getSelectedItem()->getData< db::Instance >(L"INSTANCE");

	if (!instance)
	{
		event->cancel();
		return;
	}

	if (event->getMoment() == ui::DragEvent::Moment::Drop)
	{
		Ref< IEditorPage > editorPage = m_editor->getActiveEditorPage();
		if (editorPage)
			editorPage->dropInstance(instance, event->getPosition());
	}

	event->consume();
}

void DatabaseView::eventInstancePreviewActivate(ui::MouseDoubleClickEvent* event)
{
	Ref< ui::PreviewItem > item = m_listInstances->getSelectedItem();
	if (!item)
		return;

	Ref< db::Instance > instance = item->getData< db::Instance >(L"INSTANCE");
	if (!instance)
		return;

	m_editor->openEditor(instance);
}

}

