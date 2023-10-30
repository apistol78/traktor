/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/TableLayout.h"
#include "Ui/Static.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Editor/App/SolutionPropertyPage.h"

namespace traktor::sb
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.SolutionPropertyPage", SolutionPropertyPage, ui::Container)

bool SolutionPropertyPage::create(ui::Widget* parent)
{
	if (!ui::Container::create(
		parent,
		ui::WsNone,
		new ui::TableLayout(L"*,100%", L"*", 4_ut, 4_ut)
	))
		return false;

	Ref< ui::Static > rootLabel = new ui::Static();
	rootLabel->create(this, L"Build path");

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
