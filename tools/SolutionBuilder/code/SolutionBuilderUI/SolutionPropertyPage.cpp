#include <Ui/TableLayout.h>
#include <Ui/MethodHandler.h>
#include <Ui/Static.h>
#include <Ui/Events/FocusEvent.h>
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionPropertyPage.h"
using namespace traktor;

bool SolutionPropertyPage::create(ui::Widget* parent)
{
	if (!ui::Container::create(
		parent,
		ui::WsNone,
		new ui::TableLayout(L"*,100%", L"*", 4, 4)
	))
		return false;

	Ref< ui::Static > rootLabel = new ui::Static();
	rootLabel->create(this, L"Root path");

	m_rootPath = new ui::Edit();
	m_rootPath->create(this);
	m_rootPath->addFocusEventHandler(
		new ui::MethodHandler< SolutionPropertyPage >(
			this,
			&SolutionPropertyPage::eventEditFocus
		)
	);

	return true;
}

void SolutionPropertyPage::set(Solution* solution)
{
	m_solution = solution;
	m_rootPath->setText(solution->getRootPath());
	m_rootPath->setFocus();
}

void SolutionPropertyPage::eventEditFocus(ui::Event* event)
{
	if (static_cast< ui::FocusEvent* >(event)->lostFocus())
		m_solution->setRootPath(m_rootPath->getText());
}
