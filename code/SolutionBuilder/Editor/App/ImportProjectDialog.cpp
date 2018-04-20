/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Ui/Application.h>
#include <Ui/StyleBitmap.h>
#include <Ui/TableLayout.h>
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Project.h"
#include "ImportProjectDialog.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.ImportProjectDialog", ImportProjectDialog, ui::ConfigDialog)

bool ImportProjectDialog::create(ui::Widget* parent, const std::wstring& title, bool includeDependencies, Solution* solution)
{
	if (!ui::ConfigDialog::create(
		parent,
		title,
		ui::dpi96(500),
		ui::dpi96(400),
		ui::ConfigDialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"100%,*", 4, 4)
	))
		return false;

	setIcon(new ui::StyleBitmap(L"SolutionBuilder.Icon"));

	m_listProjects = new ui::custom::ListBox();
	m_listProjects->create(this, ui::custom::ListBox::WsExtended | ui::custom::ListBox::WsSort);

	m_checkIncludeDependencies = new ui::CheckBox();
	m_checkIncludeDependencies->create(this, L"Include dependencies", false);
	m_checkIncludeDependencies->setEnable(includeDependencies);

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
	{
		Project* p = mandatory_non_null_type_cast< Project* >(m_listProjects->getData(*i));
		outProjects.push_back(p);
	}

	//if (m_checkIncludeDependencies->isChecked())
	//{
	//	for (std::vector< int >::iterator i = selected.begin(); i != selected.end(); ++i)
	//	{
	//		Project* p = mandatory_non_null_type_cast< Project* >(m_listProjects->getData(*i));

	//		p->

	//	}
	//}
}

	}
}
