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
#include "Ui/Bitmap.h"
#include "Ui/FloodLayout.h"
#include "Ui/HierarchicalState.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/TreeView.h"
#include "Ui/TreeViewItem.h"
#include "Ui/MethodHandler.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/MiniButton.h"
#include "Ui/Custom/PreviewList/PreviewItem.h"
#include "Ui/Custom/PreviewList/PreviewItems.h"
#include "Ui/Custom/PreviewList/PreviewList.h"
#include "Ui/Events/CommandEvent.h"

// Resources
#include "Resources/Files.h"

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
		640,
		500,
		ui::ConfigDialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"100%,*", 4, 4)
	))
		return false;

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	if (!splitter->create(this, true, 200))
		return false;

	Ref< ui::Container > left = new ui::Container();
	if (!left->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"22,100%", 0, 0)))
		return false;

	Ref< ui::Static > treeLabel = new ui::Static();
	if (!treeLabel->create(left, i18n::Text(L"BROWSE_INSTANCE_GROUPS")))
		return false;

	m_treeDatabase = new ui::TreeView();
	if (!m_treeDatabase->create(left, ui::WsClientBorder | ui::WsTabStop | ui::TreeView::WsTreeButtons | ui::TreeView::WsTreeLines))
		return false;
	m_treeDatabase->addImage(ui::Bitmap::load(c_ResourceFiles, sizeof(c_ResourceFiles), L"png"), 4);
	m_treeDatabase->addSelectEventHandler(ui::createMethodHandler(this, &BrowseInstanceDialog::eventTreeItemSelected));

	Ref< ui::Container > right = new ui::Container();
	if (!right->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"22,100%", 0, 0)))
		return false;

	Ref< ui::Static > listLabel = new ui::Static();
	if (!listLabel->create(right, i18n::Text(L"BROWSE_INSTANCE_INSTANCES")))
		return false;

	m_listInstances = new ui::custom::PreviewList();
	if (!m_listInstances->create(right, ui::WsClientBorder | ui::WsDoubleBuffer | ui::WsTabStop))
		return false;
	m_listInstances->addSelectEventHandler(ui::createMethodHandler(this, &BrowseInstanceDialog::eventListItemSelected));
	m_listInstances->addDoubleClickEventHandler(ui::createMethodHandler(this, &BrowseInstanceDialog::eventListDoubleClick));

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
	buildGroupItems(
		m_treeDatabase,
		0,
		database->getRootGroup(),
		filter
	);

	// Restore last state.
	Ref< ui::HierarchicalState > state = dynamic_type_cast< ui::HierarchicalState* >(m_settings->getProperty< PropertyObject >(L"Editor.BrowseInstanceTreeState"));
	if (state)
		m_treeDatabase->applyState(state);

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

void BrowseInstanceDialog::buildGroupItems(ui::TreeView* treeView, ui::TreeViewItem* parent, db::Group* group, const IBrowseFilter* filter)
{
	if (filter && !recursiveIncludeGroup(group, filter))
		return;

	RefArray< db::Group > childGroups;
	group->getChildGroups(childGroups);

	Ref< ui::TreeViewItem > groupItem = treeView->createItem(parent, group->getName(), 2, 3);
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
}

void BrowseInstanceDialog::eventTreeItemSelected(ui::Event* event)
{
	Ref< ui::TreeViewItem > item = dynamic_type_cast< ui::TreeViewItem* >(
		static_cast< ui::CommandEvent* >(event)->getItem()
	);

	// Stop all pending preview tasks.
	m_previewTasks.clear();

	if (item)
	{
		Ref< ui::custom::PreviewItems > items = item->getData< ui::custom::PreviewItems >(L"INSTANCE_ITEMS");
		m_listInstances->setItems(items);

		// Create preview generator tasks.
		if (items)
		{
			for (int i = 0; i < items->count(); ++i)
			{
				ui::custom::PreviewItem* item = items->get(i);
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

void BrowseInstanceDialog::eventListItemSelected(ui::Event* event)
{
	Ref< ui::custom::PreviewItem > item = dynamic_type_cast< ui::custom::PreviewItem* >(
		static_cast< ui::CommandEvent* >(event)->getItem()
	);

	if (item)
		m_instance = item->getData< db::Instance >(L"INSTANCE");
	else
		m_instance = 0;
}

void BrowseInstanceDialog::eventListDoubleClick(ui::Event* event)
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
