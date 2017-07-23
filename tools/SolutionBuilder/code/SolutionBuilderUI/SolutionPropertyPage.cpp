/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Ui/TableLayout.h>
#include <Ui/Static.h>
#include "SolutionBuilderUI/SolutionPropertyPage.h"
#include "SolutionBuilderLIB/Solution.h"

using namespace traktor;

bool SolutionPropertyPage::create(ui::Widget* parent)
{
	if (!ui::Container::create(
		parent,
		ui::WsNone,
		new ui::TableLayout(L"*,100%", L"*,100%", 4, 4)
	))
		return false;

	Ref< ui::Static > rootLabel = new ui::Static();
	rootLabel->create(this, L"Root path");

	m_rootPath = new ui::Edit();
	m_rootPath->create(this);
	m_rootPath->addEventHandler< ui::FocusEvent >(this, &SolutionPropertyPage::eventEditFocus);

	Ref< ui::Static > staticDefinitions = new ui::Static();
	staticDefinitions->create(this, L"Definitions");

	m_listDefinitions = new ui::custom::EditList();
	m_listDefinitions->create(this, ui::custom::EditList::WsAutoAdd | ui::custom::EditList::WsAutoRemove | ui::custom::EditList::WsSingle);
	m_listDefinitions->addEventHandler< ui::custom::EditListEditEvent >(this, &SolutionPropertyPage::eventChangeDefinitions);

	return true;
}

void SolutionPropertyPage::set(Solution* solution)
{
	m_solution = solution;

	m_rootPath->setText(m_solution->getRootPath());
	m_rootPath->setFocus();

	m_listDefinitions->removeAll();
	for (std::vector< std::wstring >::const_iterator i = m_solution->getDefinitions().begin(); i != m_solution->getDefinitions().end(); ++i)
		m_listDefinitions->add(*i);
}

void SolutionPropertyPage::eventEditFocus(ui::FocusEvent* event)
{
	if (event->lostFocus())
		m_solution->setRootPath(m_rootPath->getText());
}

void SolutionPropertyPage::eventChangeDefinitions(ui::custom::EditListEditEvent* event)
{
	std::vector< std::wstring > definitions = m_solution->getDefinitions();
	int32_t editId = event->getIndex();
	if (editId >= 0)
	{
		std::wstring text = event->getText();
		if (!text.empty())
			definitions[editId] = text;
		else
			definitions.erase(definitions.begin() + editId);
	}
	else
		definitions.push_back(event->getText());
	m_solution->setDefinitions(definitions);
	event->consume();
}
