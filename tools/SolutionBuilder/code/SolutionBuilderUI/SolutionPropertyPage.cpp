#include <Ui/TableLayout.h>
#include <Ui/MethodHandler.h>
#include <Ui/Static.h>
#include <Ui/Events/EditEvent.h>
#include <Ui/Events/FocusEvent.h>
#include "SolutionBuilderUI/EditList.h"
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
	m_rootPath->addFocusEventHandler(ui::createMethodHandler(this, &SolutionPropertyPage::eventEditFocus));

	Ref< ui::Static > staticDefinitions = new ui::Static();
	staticDefinitions->create(this, L"Definitions");

	m_listDefinitions = new EditList();
	m_listDefinitions->create(this);
	m_listDefinitions->addEditEventHandler(ui::createMethodHandler(this, &SolutionPropertyPage::eventChangeDefinitions));

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

void SolutionPropertyPage::eventEditFocus(ui::Event* event)
{
	if (static_cast< ui::FocusEvent* >(event)->lostFocus())
		m_solution->setRootPath(m_rootPath->getText());
}

void SolutionPropertyPage::eventChangeDefinitions(ui::Event* event)
{
	std::vector< std::wstring > definitions = m_solution->getDefinitions();
	int editId = static_cast< ui::EditEvent* >(event)->getParam();
	if (editId >= 0)
	{
		std::wstring text = static_cast< ui::EditEvent* >(event)->getText();
		if (!text.empty())
			definitions[editId] = text;
		else
			definitions.erase(definitions.begin() + editId);
	}
	else
		definitions.push_back(static_cast< ui::EditEvent* >(event)->getText());
	m_solution->setDefinitions(definitions);
}
