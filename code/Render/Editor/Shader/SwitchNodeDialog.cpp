/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/Shader/SwitchNodeDialog.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Ui/TableLayout.h"
#include "Ui/GridView/GridView.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridItem.h"
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
		400_ut,
		300_ut,
		ui::Dialog::WsDefaultResizable | ui::Dialog::WsCenterParent,
		new ui::TableLayout(L"100%", L"100%,*", 4_ut, 4_ut)
	))
		return false;

	m_caseList = new ui::GridView();
	m_caseList->create(this, ui::WsDoubleBuffer | ui::GridView::WsColumnHeader);
	m_caseList->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_SWITCH_CASES"), 300_ut));

	for (const auto c : switchNode->getCases())
	{
		Ref< ui::GridRow > row = new ui::GridRow();
		row->add(new ui::GridItem(toString(c)));
		m_caseList->addRow(row);
	}

	return true;
}

	}
}
