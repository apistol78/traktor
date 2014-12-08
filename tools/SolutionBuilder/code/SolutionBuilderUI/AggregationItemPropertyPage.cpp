#include <Ui/TableLayout.h>
#include <Ui/Static.h>
#include <Ui/Edit.h>
#include "SolutionBuilderLIB/AggregationItem.h"
#include "SolutionBuilderUI/AggregationItemPropertyPage.h"

using namespace traktor;

bool AggregationItemPropertyPage::create(ui::Widget* parent)
{
	if (!ui::Container::create(
		parent,
		ui::WsNone,
		new ui::TableLayout(L"*,100%", L"*,100%", 4, 4)
	))
		return false;

	Ref< ui::Static > staticTarget = new ui::Static();
	staticTarget->create(this, L"Target path");

	m_editTargetPath = new ui::Edit();
	m_editTargetPath->create(this);
	m_editTargetPath->addEventHandler< ui::FocusEvent >(this, &AggregationItemPropertyPage::eventEditFocus);

	return true;
}

void AggregationItemPropertyPage::set(AggregationItem* aggregationItem)
{
	m_aggregationItem = aggregationItem;
	m_editTargetPath->setText(m_aggregationItem->getTargetPath());
}

void AggregationItemPropertyPage::eventEditFocus(ui::FocusEvent* event)
{
	if (event->lostFocus())
	{
		m_aggregationItem->setTargetPath(m_editTargetPath->getText());

		ui::ContentChangeEvent contentChangeEvent(this);
		raiseEvent(&contentChangeEvent);
	}
}
