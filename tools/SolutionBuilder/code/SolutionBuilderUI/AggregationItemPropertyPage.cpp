#include <Ui/TableLayout.h>
#include <Ui/Static.h>
#include <Ui/Edit.h>
#include <Ui/MethodHandler.h>
#include <Ui/Events/FocusEvent.h>
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
	m_editTargetPath->addFocusEventHandler(ui::createMethodHandler(this, &AggregationItemPropertyPage::eventEditFocus));

	return true;
}

void AggregationItemPropertyPage::set(AggregationItem* aggregationItem)
{
	m_aggregationItem = aggregationItem;
	m_editTargetPath->setText(m_aggregationItem->getTargetPath());
}

void AggregationItemPropertyPage::addChangeEventHandler(ui::EventHandler* eventHandler)
{
	addEventHandler(ui::EiContentChange, eventHandler);
}

void AggregationItemPropertyPage::eventEditFocus(ui::Event* event)
{
	if (static_cast< ui::FocusEvent* >(event)->lostFocus())
	{
		m_aggregationItem->setTargetPath(m_editTargetPath->getText());
		raiseEvent(ui::EiContentChange, 0);
	}
}
