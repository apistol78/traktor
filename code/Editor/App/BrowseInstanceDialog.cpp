#include "Editor/App/BrowseInstanceDialog.h"
#include "Editor/IBrowseFilter.h"
#include "Editor/Settings.h"
#include "Ui/Bitmap.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/FloodLayout.h"
#include "Ui/TreeView.h"
#include "Ui/TreeViewItem.h"
#include "Ui/ListView.h"
#include "Ui/ListViewItem.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/MiniButton.h"
#include "I18N/Text.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"

// Resources
#include "Resources/Files.h"
#include "Resources/New.h"
#include "Resources/BigIcons.h"
#include "Resources/SmallIcons.h"

#pragma warning(disable: 4344)

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.BrowseInstanceDialog", BrowseInstanceDialog, ui::ConfigDialog)

BrowseInstanceDialog::BrowseInstanceDialog(Settings* settings)
:	m_settings(settings)
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
		gc_new< ui::TableLayout >(L"100%", L"100%,*", 4, 4)
	))
		return false;

	Ref< ui::custom::Splitter > splitter = gc_new< ui::custom::Splitter >();
	if (!splitter->create(this, true, 200))
		return false;

	Ref< ui::Container > left = gc_new< ui::Container >();
	if (!left->create(splitter, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"22,100%", 0, 0)))
		return false;

	Ref< ui::Static > treeLabel = gc_new< ui::Static >();
	if (!treeLabel->create(left, i18n::Text(L"BROWSE_INSTANCE_GROUPS")))
		return false;

	m_treeDatabase = gc_new< ui::TreeView >();
	if (!m_treeDatabase->create(left, ui::WsClientBorder | ui::TreeView::WsTreeButtons | ui::TreeView::WsTreeLines))
		return false;
	m_treeDatabase->addImage(ui::Bitmap::load(c_ResourceFiles, sizeof(c_ResourceFiles), L"png"), 4);
	m_treeDatabase->addSelectEventHandler(ui::createMethodHandler(this, &BrowseInstanceDialog::eventTreeItemSelected));

	Ref< ui::Container > right = gc_new< ui::Container >();
	if (!right->create(splitter, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"22,100%", 0, 0)))
		return false;

	Ref< ui::Container > rightTop = gc_new< ui::Container >();
	if (!rightTop->create(right, ui::WsNone, gc_new< ui::TableLayout >(L"100%,*,*", L"100%", 0, 0)))
		return false;

	Ref< ui::Static > listLabel = gc_new< ui::Static >();
	if (!listLabel->create(rightTop, i18n::Text(L"BROWSE_INSTANCE_INSTANCES")))
		return false;

	m_buttonIcon = gc_new< ui::custom::MiniButton >();
	if (!m_buttonIcon->create(rightTop, ui::Bitmap::load(c_ResourceBigIcons, sizeof(c_ResourceBigIcons), L"png")))
		return false;
	m_buttonIcon->addClickEventHandler(ui::createMethodHandler(this, &BrowseInstanceDialog::eventButtonClick));

	m_buttonSmall = gc_new< ui::custom::MiniButton >();
	if (!m_buttonSmall->create(rightTop, ui::Bitmap::load(c_ResourceSmallIcons, sizeof(c_ResourceSmallIcons), L"png")))
		return false;
	m_buttonSmall->addClickEventHandler(ui::createMethodHandler(this, &BrowseInstanceDialog::eventButtonClick));

	int32_t iconSize = m_settings->getProperty< PropertyInteger >(L"Editor.BrowseInstance.IconSize", 0);

	m_listInstances = gc_new< ui::ListView >();
	if (!m_listInstances->create(right, ui::WsClientBorder | (iconSize == 0 ? ui::ListView::WsIconNormal : ui::ListView::WsList)))
		return false;
	m_listInstances->addImage(ui::Bitmap::load(c_ResourceNew, sizeof(c_ResourceNew), L"png"), 1);
	m_listInstances->addSelectEventHandler(ui::createMethodHandler(this, &BrowseInstanceDialog::eventListItemSelected));
	m_listInstances->addDoubleClickEventHandler(ui::createMethodHandler(this, &BrowseInstanceDialog::eventListDoubleClick));

	buildGroupItems(m_treeDatabase, 0, database->getRootGroup(), filter);

	return true;
}

db::Instance* BrowseInstanceDialog::getInstance()
{
	return m_instance;
}

namespace
{

	bool recursiveIncludeGroup(db::Group* group, const IBrowseFilter* filter)
	{
		// Does this group contain a valid instance?
		for (Ref< db::Instance > instanceIter = group->getFirstChildInstance(); instanceIter; instanceIter = group->getNextChildInstance(instanceIter))
		{
			if (filter->acceptable(instanceIter))
				return true;
		}

		// No instances at this level, check if any child group contains valid instances.
		for (Ref< db::Group > groupIter = group->getFirstChildGroup(); groupIter; groupIter = group->getNextChildGroup(groupIter))
		{
			if (recursiveIncludeGroup(groupIter, filter))
				return true;
		}

		// No instances found at any level from this group and below.
		return false;
	}

}

void BrowseInstanceDialog::buildGroupItems(ui::TreeView* treeView, ui::TreeViewItem* parent, db::Group* group, const IBrowseFilter* filter)
{
	if (filter && !recursiveIncludeGroup(group, filter))
		return;

	Ref< ui::TreeViewItem > groupItem = treeView->createItem(parent, group->getName(), 2, 3);
	for (Ref< db::Group > groupIter = group->getFirstChildGroup(); groupIter; groupIter = group->getNextChildGroup(groupIter))
		buildGroupItems(treeView, groupItem, groupIter, filter);

	Ref< ui::ListViewItems > instanceItems = gc_new< ui::ListViewItems >();
	for (Ref< db::Instance > instanceIter = group->getFirstChildInstance(); instanceIter; instanceIter = group->getNextChildInstance(instanceIter))
	{
		if (!filter || filter->acceptable(instanceIter))
		{
			Ref< ui::ListViewItem > instanceItem = gc_new< ui::ListViewItem >();
			instanceItem->setImage(0, 0);
			instanceItem->setText(0, instanceIter->getName());
			instanceItem->setData(L"INSTANCE", instanceIter);
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
	if (item)
	{
		Ref< ui::ListViewItems > items = item->getData< ui::ListViewItems >(L"INSTANCE_ITEMS");
		m_listInstances->setItems(items);
	}
	else
		m_listInstances->setItems(0);
}

void BrowseInstanceDialog::eventListItemSelected(ui::Event* event)
{
	Ref< ui::ListViewItem > item = dynamic_type_cast< ui::ListViewItem* >(
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

void BrowseInstanceDialog::eventButtonClick(ui::Event* event)
{
	if (event->getSender() == m_buttonIcon)
	{
		m_listInstances->setStyle(ui::ListView::WsIconNormal | ui::WsClientBorder);
		m_settings->setProperty< PropertyInteger >(L"Editor.BrowseInstance.IconSize", 0);
	}
	else
	{
		m_listInstances->setStyle(ui::ListView::WsList | ui::WsClientBorder);
		m_settings->setProperty< PropertyInteger >(L"Editor.BrowseInstance.IconSize", 1);
	}
}

	}
}
