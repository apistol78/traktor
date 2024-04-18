/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyObject.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadPool.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/IBrowseFilter.h"
#include "Editor/IBrowsePreview.h"
#include "Editor/App/BrowseInstanceDialog.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/HierarchicalState.h"
#include "Ui/Static.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Splitter.h"
#include "Ui/MiniButton.h"
#include "Ui/PreviewList/PreviewItem.h"
#include "Ui/PreviewList/PreviewItems.h"
#include "Ui/PreviewList/PreviewList.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewItem.h"

// Resources
#include "Resources/Folders.h"

#pragma warning(disable: 4344)

namespace traktor::editor
{
	namespace
	{

bool recursiveIncludeGroup(db::Group* group, const IBrowseFilter* filter)
{
	RefArray< db::Instance > childInstances;
	group->getChildInstances(childInstances);

	// Does this group contain a valid instance?
	for (auto childInstance : childInstances)
	{
		if (filter->acceptable(childInstance))
			return true;
	}

	RefArray< db::Group > childGroups;
	group->getChildGroups(childGroups);

	// No instances at this level, check if any child group contains valid instances.
	for (auto childGroup : childGroups)
	{
		if (recursiveIncludeGroup(childGroup, filter))
			return true;
	}

	// No instances found at any level from this group and below.
	return false;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.BrowseInstanceDialog", BrowseInstanceDialog, ui::ConfigDialog)

BrowseInstanceDialog::BrowseInstanceDialog(IEditor* editor, PropertyGroup* settings)
:	m_editor(editor)
,	m_settings(settings)
,	m_threadGeneratePreview(nullptr)
{
}

bool BrowseInstanceDialog::create(ui::Widget* parent, db::Database* database, const IBrowseFilter* filter)
{
	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"BROWSE_INSTANCE_TITLE"),
		640_ut,
		500_ut,
		ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"100%,*", 4_ut, 4_ut)
	))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	Ref< ui::Splitter > splitter = new ui::Splitter();
	if (!splitter->create(this, true, 200_ut))
		return false;

	Ref< ui::Container > left = new ui::Container();
	if (!left->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut)))
		return false;

	Ref< ui::Static > treeLabel = new ui::Static();
	if (!treeLabel->create(left, i18n::Text(L"BROWSE_INSTANCE_GROUPS")))
		return false;

	m_treeDatabase = new ui::TreeView();
	if (!m_treeDatabase->create(left, ui::WsDoubleBuffer | ui::WsTabStop))
		return false;
	m_treeDatabase->addImage(new ui::StyleBitmap(L"Editor.Database.Folders", 0));
	m_treeDatabase->addImage(new ui::StyleBitmap(L"Editor.Database.Folders", 1));
	m_treeDatabase->addEventHandler< ui::SelectionChangeEvent >(this, &BrowseInstanceDialog::eventTreeItemSelected);

	Ref< ui::Container > right = new ui::Container();
	if (!right->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut)))
		return false;

	Ref< ui::Static > listLabel = new ui::Static();
	if (!listLabel->create(right, i18n::Text(L"BROWSE_INSTANCE_INSTANCES")))
		return false;

	m_listInstances = new ui::PreviewList();
	if (!m_listInstances->create(right, ui::WsDoubleBuffer | ui::WsTabStop))
		return false;
	m_listInstances->addEventHandler< ui::PreviewSelectionChangeEvent >(this, &BrowseInstanceDialog::eventListItemSelected);
	m_listInstances->addEventHandler< ui::MouseDoubleClickEvent >(this, &BrowseInstanceDialog::eventListDoubleClick);

	// Create browse preview generators.
	for (auto typeInfo : type_of< IBrowsePreview >().findAllOf(false))
	{
		Ref< const IBrowsePreview > browsePreview = checked_type_cast< const IBrowsePreview*, false >(typeInfo->createInstance());
		m_browsePreview.push_back(browsePreview);
	}

	// Spawn preview generator thread.
	ThreadPool::getInstance().spawn(
		[this](){ threadGeneratePreview(); },
		m_threadGeneratePreview
	);

	// Traverse database and filter out items.
	ui::TreeViewItem* groupRoot = buildGroupItems(
		m_treeDatabase,
		nullptr,
		database->getRootGroup(),
		filter
	);

	if (groupRoot)
		groupRoot->sort(true);

	// Expand all groups until a group with multiple children is found.
	ui::TreeViewItem* expandGroup = groupRoot;
	while (expandGroup)
	{
		expandGroup->expand();

		const RefArray< ui::TreeViewItem >& children = expandGroup->getChildren();
		if (children.size() == 1)
			expandGroup = children[0];
		else
			break;
	}

	// Restore last state.
	Ref< ui::HierarchicalState > state = dynamic_type_cast< ui::HierarchicalState* >(m_settings->getProperty< Ref< ISerializable > >(L"Editor.BrowseInstanceTreeState"));
	if (state)
		m_treeDatabase->applyState(state);

	updatePreviewList();
	return true;
}

void BrowseInstanceDialog::destroy()
{
	if (m_threadGeneratePreview)
	{
		ThreadPool::getInstance().stop(m_threadGeneratePreview);
		m_threadGeneratePreview = nullptr;
	}

	m_previewTasks.clear();

	if (m_settings)
	{
		Ref< ui::HierarchicalState > state = m_treeDatabase->captureState();
		m_settings->setProperty< PropertyObject >(L"Editor.BrowseInstanceTreeState", state);
	}

	ui::ConfigDialog::destroy();
}

Ref< db::Instance > BrowseInstanceDialog::getInstance()
{
	return m_instance;
}

ui::TreeViewItem* BrowseInstanceDialog::buildGroupItems(ui::TreeView* treeView, ui::TreeViewItem* parent, db::Group* group, const IBrowseFilter* filter)
{
	if (filter && !recursiveIncludeGroup(group, filter))
		return nullptr;

	RefArray< db::Group > childGroups;
	group->getChildGroups(childGroups);

	Ref< ui::TreeViewItem > groupItem = treeView->createItem(parent, group->getName(), 1);
	groupItem->setImage(0, 0, 1);

	for (auto childGroup : childGroups)
		buildGroupItems(treeView, groupItem, childGroup, filter);

	RefArray< db::Instance > childInstances;
	group->getChildInstances(childInstances);

	Ref< ui::PreviewItems > instanceItems = new ui::PreviewItems();
	for (auto childInstance : childInstances)
	{
		if (!filter || filter->acceptable(childInstance))
		{
			Ref< ui::PreviewItem > instanceItem = new ui::PreviewItem(childInstance->getName());
			instanceItem->setData(L"INSTANCE", childInstance);
			instanceItems->add(instanceItem);
		}
	}

	groupItem->setData(L"INSTANCE_ITEMS", instanceItems);
	return groupItem;
}

void BrowseInstanceDialog::updatePreviewList()
{
	RefArray< ui::TreeViewItem > items;
	m_treeDatabase->getItems(items, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly);

	// Stop all pending preview tasks.
	m_previewTasks.clear();

	if (!items.empty())
	{
		Ref< ui::PreviewItems > previewItems = items[0]->getData< ui::PreviewItems >(L"INSTANCE_ITEMS");
		m_listInstances->setItems(previewItems);

		// Create preview generator tasks.
		if (previewItems)
		{
			for (int i = 0; i < previewItems->count(); ++i)
			{
				ui::PreviewItem* item = previewItems->get(i);
				if (!item->getImage())
				{
					m_previewTasks.put([=](){ taskGeneratePreview(item); });
					m_previewTaskEvent.pulse();
				}
			}
		}
	}
	else
		m_listInstances->setItems(nullptr);
}

void BrowseInstanceDialog::eventTreeItemSelected(ui::SelectionChangeEvent* event)
{
	updatePreviewList();
}

void BrowseInstanceDialog::eventListItemSelected(ui::PreviewSelectionChangeEvent* event)
{
	Ref< ui::PreviewItem > item = event->getItem();
	if (item)
		m_instance = item->getData< db::Instance >(L"INSTANCE");
	else
		m_instance = nullptr;
}

void BrowseInstanceDialog::eventListDoubleClick(ui::MouseDoubleClickEvent* event)
{
	if (m_instance)
		endModal(ui::DialogResult::Ok);
}

void BrowseInstanceDialog::taskGeneratePreview(ui::PreviewItem* item)
{
	Ref< db::Instance > instance = item->getData< db::Instance >(L"INSTANCE");
	T_ASSERT(instance);

	const TypeInfo* instanceType = instance->getPrimaryType();
	for (RefArray< const IBrowsePreview >::const_iterator i = m_browsePreview.begin(); i != m_browsePreview.end(); ++i)
	{
		TypeInfoSet previewTypes = (*i)->getPreviewTypes();
		if (previewTypes.find(instanceType) != previewTypes.end())
		{
			item->setImage((*i)->generate(
				m_editor,
				instance
			));
			m_listInstances->requestUpdate();
			break;
		}
	}
}

void BrowseInstanceDialog::threadGeneratePreview()
{
	std::function< void() > task;
	while (m_threadGeneratePreview && !m_threadGeneratePreview->stopped())
	{
		if (!m_previewTasks.get(task))
		{
			m_previewTaskEvent.wait(100);
			continue;
		}
		task();
	}
}

}
