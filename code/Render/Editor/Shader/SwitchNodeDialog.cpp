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
		400,
		300,
		ui::Dialog::WsDefaultResizable | ui::Dialog::WsCenterParent,
		new ui::TableLayout(L"100%", L"100%,*", 4, 4)
	))
		return false;

	m_caseList = new ui::GridView();
	m_caseList->create(this, ui::WsClientBorder | ui::WsDoubleBuffer | ui::GridView::WsColumnHeader);
	m_caseList->addColumn(new ui::GridColumn(i18n::Text(L"SHADERGRAPH_SWITCH_CASES"), 300));

	const std::vector< int32_t >& cases = switchNode->getCases();
	for (std::vector< int32_t >::const_iterator i = cases.begin(); i != cases.end(); ++i)
	{
		Ref< ui::GridRow > row = new ui::GridRow();
		row->add(new ui::GridItem(toString(*i)));
		m_caseList->addRow(row);
	}

	return true;
}

	}
}
