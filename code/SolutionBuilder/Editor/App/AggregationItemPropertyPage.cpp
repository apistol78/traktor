/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Ui/Application.h>
#include <Ui/TableLayout.h>
#include <Ui/Static.h>
#include <Ui/Edit.h>
#include "SolutionBuilder/AggregationItem.h"
#include "SolutionBuilder/Editor/App/AggregationItemPropertyPage.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.AggregationItemPropertyPage", AggregationItemPropertyPage, ui::Container)

bool AggregationItemPropertyPage::create(ui::Widget* parent)
{
	const int32_t f = ui::dpi96(4);

	if (!ui::Container::create(
		parent,
		ui::WsNone,
		new ui::TableLayout(L"*,100%", L"*,*,100%", f, f)
	))
		return false;

	Ref< ui::Static > staticSource = new ui::Static();
	staticSource->create(this, L"Source file");

	m_editSourceFile = new ui::Edit();
	m_editSourceFile->create(this);
	m_editSourceFile->addEventHandler< ui::FocusEvent >(this, &AggregationItemPropertyPage::eventEditFocus);

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
	m_editSourceFile->setText(m_aggregationItem->getSourceFile());
	m_editTargetPath->setText(m_aggregationItem->getTargetPath());
}

void AggregationItemPropertyPage::eventEditFocus(ui::FocusEvent* event)
{
	if (event->lostFocus())
	{
		m_aggregationItem->setSourceFile(m_editSourceFile->getText());
		m_aggregationItem->setTargetPath(m_editTargetPath->getText());

		ui::ContentChangeEvent contentChangeEvent(this);
		raiseEvent(&contentChangeEvent);
	}
}

	}
}
