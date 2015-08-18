#include <Ui/Application.h>
#include <Ui/TableLayout.h>
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "ImportProjectDialog.h"

using namespace traktor;

bool ImportProjectDialog::create(ui::Widget* parent, const std::wstring& title, Solution* solution)
{
	if (!ui::ConfigDialog::create(
		parent,
		title,
		ui::scaleBySystemDPI(500),
		ui::scaleBySystemDPI(400),
		ui::ConfigDialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"100%", 4, 4)
	))
		return false;

	m_listProjects = new ui::ListBox();
	m_listProjects->create(this, L"", ui::WsClientBorder | ui::ListBox::WsExtended);

	const RefArray< Project >& projects = solution->getProjects();
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
		m_listProjects->add((*i)->getName(), *i);

	return true;
}

void ImportProjectDialog::getSelectedProjects(RefArray< Project >& outProjects)
{
	std::vector< int > selected;
	m_listProjects->getSelected(selected);
	for (std::vector< int >::iterator i = selected.begin(); i != selected.end(); ++i)
		outProjects.push_back(checked_type_cast< Project*, false >(m_listProjects->getData(*i)));
}
