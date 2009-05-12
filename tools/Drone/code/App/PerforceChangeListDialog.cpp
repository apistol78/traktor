#include <Ui/TableLayout.h>
#include <Ui/Custom/GridView/GridView.h>
#include <Ui/Custom/GridView/GridColumn.h>
#include <Ui/Custom/GridView/GridRow.h>
#include <Ui/Custom/GridView/GridItem.h>
#include <Core/Misc/StringUtils.h>
#include "App/PerforceChangeListDialog.h"
#include "App/PerforceChangeList.h"
#include "App/PerforceChangeListFile.h"

namespace traktor
{
	namespace drone
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drone.PerforceChangeListDialog", PerforceChangeListDialog, ui::ConfigDialog)

bool PerforceChangeListDialog::create(ui::Widget* parent, const std::wstring& text, const RefArray< PerforceChangeList >& changeLists)
{
	if (!ui::ConfigDialog::create(
		parent,
		text,
		700,
		600,
		ui::ConfigDialog::WsDefaultResizable | ui::ConfigDialog::WsCenterDesktop,
		gc_new< ui::TableLayout >(L"100%", L"100%", 4, 4)
	))
		return false;

	m_changeLists = gc_new< ui::custom::GridView >();
	m_changeLists->create(this, ui::custom::GridView::WsColumnHeader | ui::WsClientBorder | ui::WsDoubleBuffer);
	m_changeLists->addColumn(gc_new< ui::custom::GridColumn >(L"User", 100));
	m_changeLists->addColumn(gc_new< ui::custom::GridColumn >(L"Change list", 100));
	m_changeLists->addColumn(gc_new< ui::custom::GridColumn >(L"Description", 400));

	std::map< std::wstring, RefArray< PerforceChangeList > > userChangeLists;
	for (RefArray< PerforceChangeList >::const_iterator i = changeLists.begin(); i != changeLists.end(); ++i)
		userChangeLists[(*i)->getUser()].push_back(*i);

	for (std::map< std::wstring, RefArray< PerforceChangeList > >::iterator i = userChangeLists.begin(); i != userChangeLists.end(); ++i)
	{
		Ref< ui::custom::GridRow > userRow = gc_new< ui::custom::GridRow >();
		userRow->addItem(gc_new< ui::custom::GridItem >(i->first));

		for (RefArray< PerforceChangeList >::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			Ref< ui::custom::GridRow > changeRow = gc_new< ui::custom::GridRow >();
			changeRow->addItem(gc_new< ui::custom::GridItem >(L""));
			changeRow->addItem(gc_new< ui::custom::GridItem >(toString((*j)->getChange())));
			changeRow->addItem(gc_new< ui::custom::GridItem >((*j)->getDescription()));
			changeRow->setData(L"CHANGELIST", *j);

			const RefArray< PerforceChangeListFile >& changeListFiles = (*j)->getFiles();
			for (RefArray< PerforceChangeListFile >::const_iterator k = changeListFiles.begin(); k != changeListFiles.end(); ++k)
			{
				const wchar_t* c_actions[] = { L"", L"(add)", L"(edit)", L"(delete)" };

				Ref< ui::custom::GridRow > fileRow = gc_new< ui::custom::GridRow >();
				fileRow->addItem(gc_new< ui::custom::GridItem >(L""));
				fileRow->addItem(gc_new< ui::custom::GridItem >(c_actions[(*k)->getAction()]));
				fileRow->addItem(gc_new< ui::custom::GridItem >((*k)->getDepotPath()));
				changeRow->addChild(fileRow);
			}

			changeRow->setState(0);

			userRow->addChild(changeRow);
		}

		m_changeLists->addRow(userRow);
	}

	return true;
}

void PerforceChangeListDialog::getSelectedChangeLists(RefArray< PerforceChangeList >& outSelectedChangeLists) const
{
	RefArray< ui::custom::GridRow > selectedRows;
	m_changeLists->getRows(selectedRows, ui::custom::GridView::GfSelectedOnly | ui::custom::GridView::GfDescendants);

	outSelectedChangeLists.resize(0);
	for (RefArray< ui::custom::GridRow >::iterator i = selectedRows.begin(); i != selectedRows.end(); ++i)
	{
		Ref< PerforceChangeList > changeList = (*i)->getData< PerforceChangeList >(L"CHANGELIST");
		if (changeList)
			outSelectedChangeLists.push_back(changeList);
	}
}

	}
}
