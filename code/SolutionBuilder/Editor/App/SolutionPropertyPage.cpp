/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Ui/Application.h>
#include <Ui/TableLayout.h>
#include <Ui/Static.h>
#include "SolutionBuilder/Editor/App/SolutionPropertyPage.h"
#include "SolutionBuilder/Solution.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.SolutionPropertyPage", SolutionPropertyPage, ui::Container)

bool SolutionPropertyPage::create(ui::Widget* parent)
{
	const int32_t f = ui::dpi96(4);

	if (!ui::Container::create(
		parent,
		ui::WsNone,
		new ui::TableLayout(L"*,100%", L"*", f, f)
	))
		return false;

	Ref< ui::Static > rootLabel = new ui::Static();
	rootLabel->create(this, L"Root path");

	m_rootPath = new ui::Edit();
	m_rootPath->create(this);
	m_rootPath->addEventHandler< ui::FocusEvent >(this, &SolutionPropertyPage::eventEditFocus);

	Ref< ui::Static > staticAggregateOutputPath = new ui::Static();
	staticAggregateOutputPath->create(this, L"Aggregate output path");

	m_aggregateOutputPath = new ui::Edit();
	m_aggregateOutputPath->create(this);
	m_aggregateOutputPath->addEventHandler< ui::FocusEvent >(this, &SolutionPropertyPage::eventEditFocus);

	return true;
}

void SolutionPropertyPage::set(Solution* solution)
{
	m_solution = solution;

	m_rootPath->setText(m_solution->getRootPath());
	m_aggregateOutputPath->setText(m_solution->getAggregateOutputPath());

	m_rootPath->setFocus();
}

void SolutionPropertyPage::eventEditFocus(ui::FocusEvent* event)
{
	if (!event->lostFocus())
		return;

	m_solution->setRootPath(m_rootPath->getText());
	m_solution->setAggregateOutputPath(m_aggregateOutputPath->getText());
}

	}
}
