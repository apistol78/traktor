/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/Edit.h"
#include "Ui/FloodLayout.h"
#include "Ui/Static.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/ListBox/ListBox.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/Editor/App/ExtractSolutionDialog.h"

namespace traktor
{
	namespace sb
	{
		namespace
		{

struct ProjectSortPredicate
{
	bool operator () (const Project* p1, const Project* p2) const
	{
		return p1->getName().compare(p2->getName()) < 0;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.ExtractSolutionDialog", ExtractSolutionDialog, ui::ConfigDialog)

bool ExtractSolutionDialog::create(ui::Widget* parent, Solution* solution)
{
	if (!ui::ConfigDialog::create(
		parent,
		L"Extract project(s) into solution",
		ui::dpi96(1000),
		ui::dpi96(600),
		ui::ConfigDialog::WsDefaultResizable,
		new ui::FloodLayout()
	))
		return false;

	setIcon(new ui::StyleBitmap(L"SolutionBuilder.Icon"));

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(this, true, ui::dpi96(300));

	m_listProjects = new ui::custom::ListBox();
	m_listProjects->create(splitter, ui::custom::ListBox::WsMultiple);

	Ref< ui::Container > container = new ui::Container();
	container->create(splitter, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", ui::dpi96(8), ui::dpi96(4)));

	Ref< ui::Static > staticSolutionFile = new ui::Static();
	staticSolutionFile->create(container, L"Solution file");

	m_editSolutionFile = new ui::Edit();
	m_editSolutionFile->create(container);

	Ref< ui::Static > staticSolutionName = new ui::Static();
	staticSolutionName->create(container, L"Solution name");

	m_editSolutionName = new ui::Edit();
	m_editSolutionName->create(container);

	RefArray< Project > projects = solution->getProjects();
	projects.sort(ProjectSortPredicate());

	for (auto project : projects)
		m_listProjects->add(project->getName(), project);

	return true;
}

void ExtractSolutionDialog::getSelectedProjects(RefArray< Project >& outProjects) const
{
	std::vector< int32_t > ids;
	m_listProjects->getSelected(ids);

	for (auto id : ids)
	{
		Project* project = m_listProjects->getData< Project >(id);
		T_ASSERT (project != nullptr);

		outProjects.push_back(project);
	}
}

std::wstring ExtractSolutionDialog::getSolutionFile() const
{
	return m_editSolutionFile->getText();
}

std::wstring ExtractSolutionDialog::getSolutionName() const
{
	return m_editSolutionName->getText();
}

	}
}
