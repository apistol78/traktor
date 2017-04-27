/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Editor/Shader/SwitchNodeDialog.h"
#include "Render/Shader/Nodes.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "I18N/Text.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SwitchNodeDialog", SwitchNodeDialog, ui::ConfigDialog)

bool SwitchNodeDialog::create(ui::Widget* parent, Switch* switchNode)
{
	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"SHADERGRAPH_SWITCH_EDIT_TITLE"),
		400,
		300,
		ui::Dialog::WsDefaultResizable | ui::Dialog::WsCenterParent,
		new ui::TableLayout(L"100%", L"100%,*", 4, 4)
	))
		return false;

	m_caseList = new ui::custom::GridView();
	m_caseList->create(this, ui::WsClientBorder | ui::WsDoubleBuffer | ui::custom::GridView::WsColumnHeader);
	m_caseList->addColumn(new ui::custom::GridColumn(i18n::Text(L"SHADERGRAPH_SWITCH_CASES"), 300));

	const std::vector< int32_t >& cases = switchNode->getCases();
	for (std::vector< int32_t >::const_iterator i = cases.begin(); i != cases.end(); ++i)
	{
		Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem(toString(*i)));
		m_caseList->addRow(row);
	}

	return true;
}

	}
}
