/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Functor/Functor.h"
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
#include "Ui/FloodLayout.h"
#include "Ui/HierarchicalState.h"
#include "Ui/Static.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/Custom/PreviewList/PreviewItem.h"
#include "Ui/Custom/PreviewList/PreviewItems.h"
#include "Ui/Custom/PreviewList/PreviewList.h"
#include "Ui/Custom/TreeView/TreeView.h"
#include "Ui/Custom/TreeView/TreeViewItem.h"

// Resources
#include "Resources/Folders.h"

#pragma warning(disable: 4344)

namespace traktor
{
	namespace editor
	{
		namespace
		{

bool recursiveIncludeGroup(db::Group* group, const IBrowseFilter* filter)
{
	RefArray< db::Instance > childInstances;
	group->getChildInstances(childInstances);

	// Does this group contain a valid instance?
	for (RefArray< db::Instance >::iterator i = childInstances.begin(); i != childInstances.end(); ++i)
	{
		if (filter->acceptable(*i))
			return true;
	}

	RefArray< db::Group > childGroups;
	group->getChildGroups(childGroups);

	// No instances at this level, check if any child group contains valid instances.
	for (RefArray< db::Group >::iterator i = childGroups.begin(); i != childGroups.end(); ++i)
	{
		if (recursiveIncludeGroup(*i, filter))
			return true;
	}

	// No instances found at any level from this group and below.
	return false;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.BrowseInstanceDialog", BrowseInstanceDialog, ui::ConfigDialog)

BrowseInstanceDialog::BrowseInstanceDialog(const IEditor* editor, PropertyGroup* settings)
:	m_editor(editor)
,	m_settings(settings)
,	m_threadGeneratePreview(0)
{
}

bool BrowseInstanceDialog::create(ui::Widget* parent, db::Database* database, const IBrowseFilter* filter)
{
	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"BROWSE_INSTANCE_TITLE"),
		ui::scaleBySystemDPI(640),
		ui::scaleBySystemDPI(500),
		ui::ConfigDialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"100%,*", 4, 4)
	))
		return false;

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	if (!splitter->create(this, true, ui::scaleBySystemDPI(200)))
		return false;

	Ref< ui::Container > left = new ui::Container();
	if (!left->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	Ref< ui::Static > treeLabel = new ui::Static();
	if (!treeLabel->create(left, i18n::Text(L"BROWSE_INSTANCE_GROUPS")))
		return false;

	m_treeDatabase = new ui::custom::TreeView();
	if (!m_treeDatabase->create(left, ui::WsDoubleBuffer | ui::WsTabStop))
		return false;
	m_treeDatabase->addImage(new ui::StyleBitmap(L"Editor.Database.Folders"), 2);
	m_treeDatabase->addEventHandler< ui::SelectionChangeEvent >(this, &BrowseInstanceDialog::eventTreeItemSelected);

	Ref< ui::Container > right = new ui::Container();
	if (!right->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	Ref< ui::Static > listLabel = new ui::Static();
	if (!listLabel->create(right, i18n::Text(L"BROWSE_INSTANCE_INSTANCES")))
		return false;

	m_listInstances = new ui::custom::PreviewList();
	if (!m_listInstances->create(right, ui::WsDoubleBuffer | ui::WsTabStop))
		return false;
	m_listInstances->addEventHandler< ui::custom::PreviewSelectionChangeEvent >(this, &BrowseInstanceDialog::eventListItemSelected);
	m_listInstances->addEventHandler< ui::MouseDoubleClickEvent >(this, &BrowseInstanceDialog::eventListDoubleClick);

	// Create browse preview generators.
	TypeInfoSet browsePreviewImplTypes;
	type_of< IBrowsePreview >().findAllOf(browsePreviewImplTypes, false);
	for (TypeInfoSet::const_iterator i = browsePreviewImplTypes.begin(); i != browsePreviewImplTypes.end(); ++i)
	{
		Ref< const IBrowsePreview > browsePreview = checked_type_cast< const IBrowsePreview*, false >((*i)->createInstance());
		m_browsePreview.push_back(browsePreview);
	}

	// Spawn preview generator thread.
	ThreadPool::getInstance().spawn(
		makeFunctor(this, &BrowseInstanceDialog::threadGeneratePreview),
		m_threadGeneratePreview
	);

	// Traverse database and filter out items.
	ui::custom::TreeViewItem* groupRoot = buildGroupItems(
		m_treeDatabase,
		0,
		database->getRootGroup(),
		filter
	);

	if (groupRoot)
		groupRoot->sort(true);

	// Expand all groups until a group with multiple children is found.
	ui::custom::TreeViewItem* expandGroup = groupRoot;
	while (expandGroup)
	{
		expandGroup->expand();

		const RefArray< ui::custom::TreeViewItem >& children = expandGroup->getChildren();
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
		m_threadGeneratePreview = 0;
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

ui::custom::TreeViewItem* BrowseInstanceDialog::buildGroupItems(ui::custom::TreeView* treeView, ui::custom::TreeViewItem* parent, db::Group* group, const IBrowseFilter* filter)
{
	if (filter && !recursiveIncludeGroup(group, filter))
		return 0;

	RefArray< db::Group > childGroups;
	group->getChildGroups(childGroups);

	Ref< ui::custom::TreeViewItem > groupItem = treeView->createItem(parent, group->getName(), 0, 1);
	for (RefArray< db::Group >::iterator i = childGroups.begin(); i != childGroups.end(); ++i)
		buildGroupItems(treeView, groupItem, *i, filter);

	RefArray< db::Instance > childInstances;
	group->getChildInstances(childInstances);

	Ref< ui::custom::PreviewItems > instanceItems = new ui::custom::PreviewItems();
	for (RefArray< db::Instance >::iterator i = childInstances.begin(); i != childInstances.end(); ++i)
	{
		if (!filter || filter->acceptable(*i))
		{
			Ref< ui::custom::PreviewItem > instanceItem = new ui::custom::PreviewItem((*i)->getName());
			instanceItem->setData(L"INSTANCE", (*i));
			instanceItems->add(instanceItem);
		}
	}

	groupItem->setData(L"INSTANCE_ITEMS", instanceItems);
	return groupItem;
}

void BrowseInstanceDialog::updatePreviewList()
{
	RefArray< ui::custom::TreeViewItem > items;
	m_treeDatabase->getItems(items, ui::custom::TreeView::GfDescendants | ui::custom::TreeView::GfSelectedOnly);

	// Stop all pending preview tasks.
	m_previewTasks.clear();

	if (!items.empty())
	{
		Ref< ui::custom::PreviewItems > previewItems = items[0]->getData< ui::custom::PreviewItems >(L"INSTANCE_ITEMS");
		m_listInstances->setItems(previewItems);

		// Create preview generator tasks.
		if (previewItems)
		{
			for (int i = 0; i < previewItems->count(); ++i)
			{
				ui::custom::PreviewItem* item = previewItems->get(i);
				if (!item->getImage())
				{
					m_previewTasks.put(makeFunctor(this, &BrowseInstanceDialog::taskGeneratePreview, item));
					m_previewTaskEvent.pulse();
				}
			}
		}
	}
	else
		m_listInstances->setItems(0);
}

void BrowseInstanceDialog::eventTreeItemSelected(ui::SelectionChangeEvent* event)
{
	updatePreviewList();
}

void BrowseInstanceDialog::eventListItemSelected(ui::custom::PreviewSelectionChangeEvent* event)
{
	Ref< ui::custom::PreviewItem > item = event->getItem();
	if (item)
		m_instance = item->getData< db::Instance >(L"INSTANCE");
	else
		m_instance = 0;
}

void BrowseInstanceDialog::eventListDoubleClick(ui::MouseDoubleClickEvent* event)
{
	if (m_instance)
		endModal(ui::DrOk);
}

void BrowseInstanceDialog::taskGeneratePreview(ui::custom::PreviewItem* item)
{
	Ref< db::Instance > instance = item->getData< db::Instance >(L"INSTANCE");
	T_ASSERT (instance);

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
	Ref< Functor > task;
	while (m_threadGeneratePreview && !m_threadGeneratePreview->stopped())
	{
		if (!m_previewTasks.get(task))
		{
			m_previewTaskEvent.wait(100);
			continue;
		}
		(*task)(); task = 0;
	}
}

	}
}
