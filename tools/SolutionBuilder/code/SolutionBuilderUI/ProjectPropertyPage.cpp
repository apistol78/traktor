/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Ui/Application.h>
#include <Ui/TableLayout.h>
#include <Ui/Static.h>
#include <Ui/Button.h>
#include <Ui/ListViewItem.h>
#include <Ui/FileDialog.h>
#include <Ui/MessageBox.h>
#include <Ui/Custom/InputDialog.h>
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/SolutionLoader.h"
#include "ProjectPropertyPage.h"
#include "ImportProjectDialog.h"
using namespace traktor;

namespace
{

	struct DependencyPredicate
	{
		bool operator () (const Dependency* dep1, const Dependency* dep2) const
		{
			return dep1->getName().compare(dep2->getName()) < 0;
		}
	};

	struct ProjectSortPredicate
	{
		bool operator () (const Project* p1, const Project* p2) const
		{
			return p1->getName().compare(p2->getName()) < 0;
		}
	};

}

bool ProjectPropertyPage::create(ui::Widget* parent)
{
	if (!ui::Container::create(
		parent,
		ui::WsNone,
		new ui::TableLayout(L"100%", L"*,100%", 4, 4)
	))
		return false;

	m_checkEnable = new ui::CheckBox();
	m_checkEnable->create(this, L"Include project in build");
	m_checkEnable->addEventHandler< ui::ButtonClickEvent >(this, &ProjectPropertyPage::eventEnableClick);

	Ref< ui::Container > container = new ui::Container();
	container->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*,100%,*", 0, 4));

	Ref< ui::Static > staticSourcePath = new ui::Static();
	staticSourcePath->create(container, L"Source path");

	m_editSourcePath = new ui::Edit();
	m_editSourcePath->create(container);
	m_editSourcePath->addEventHandler< ui::FocusEvent >(this, &ProjectPropertyPage::eventFocusSource);

	Ref< ui::Static > staticDependencies = new ui::Static();
	staticDependencies->create(container, L"Dependencies");

	m_listDependencies = new ui::ListView();
	m_listDependencies->create(container, ui::WsClientBorder | ui::ListView::WsReport);
	m_listDependencies->addColumn(L"Dependency", ui::scaleBySystemDPI(130));
	m_listDependencies->addColumn(L"Location", ui::scaleBySystemDPI(200));
	m_listDependencies->addColumn(L"Inherit include paths", ui::scaleBySystemDPI(100));
	m_listDependencies->addColumn(L"Link", ui::scaleBySystemDPI(50));
	m_listDependencies->addEventHandler< ui::MouseDoubleClickEvent >(this, &ProjectPropertyPage::eventDependencyDoubleClick);

	Ref< ui::Static > staticAvailable = new ui::Static();
	staticAvailable->create(container, L"Available");

	Ref< ui::Container > containerAvailable = new ui::Container();
	containerAvailable->create(container, ui::WsNone, new ui::TableLayout(L"100%,*,*,*", L"*", 0, 4));

	m_dropAvailable = new ui::DropDown();
	m_dropAvailable->create(containerAvailable);

	Ref< ui::Button > buttonAdd = new ui::Button();
	buttonAdd->create(containerAvailable, L"Add");
	buttonAdd->addEventHandler< ui::ButtonClickEvent >(this, &ProjectPropertyPage::eventClickAdd);

	Ref< ui::Button > buttonRemove = new ui::Button();
	buttonRemove->create(containerAvailable, L"Remove");
	buttonRemove->addEventHandler< ui::ButtonClickEvent >(this, &ProjectPropertyPage::eventClickRemove);

	Ref< ui::Button > buttonAddExternal = new ui::Button();
	buttonAddExternal->create(containerAvailable, L"External...");
	buttonAddExternal->addEventHandler< ui::ButtonClickEvent >(this, &ProjectPropertyPage::eventClickAddExternal);

	return true;
}

void ProjectPropertyPage::set(Solution* solution, Project* project)
{
	m_solution = solution;
	m_project = project;

	m_checkEnable->setChecked(project->getEnable());
	m_editSourcePath->setText(project->getSourcePath());

	updateDependencyList();
}

void ProjectPropertyPage::updateDependencyList()
{
	RefArray< Dependency > dependencies = m_project->getDependencies();
	Ref< ui::ListViewItems > dependencyItems = new ui::ListViewItems();

	const wchar_t* c_link[] = { L"No", L"Yes", L"Force" };

	// Sort all dependencies.
	dependencies.sort(DependencyPredicate());

	// Add all local dependencies first.
	for (RefArray< Dependency >::iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		if (is_a< ExternalDependency >(*i))
			continue;

		Ref< ui::ListViewItem > dependencyItem = new ui::ListViewItem();
		dependencyItem->setText(0, (*i)->getName());
		dependencyItem->setText(1, (*i)->getLocation());
		dependencyItem->setText(2, (*i)->getInheritIncludePaths() ? L"Yes" : L"No");
		dependencyItem->setText(3, c_link[(*i)->getLink()]);
		dependencyItem->setData(L"DEPENDENCY", *i);
		dependencyItems->add(dependencyItem);
	}

	// Add external dependencies last.
	for (RefArray< Dependency >::iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		if (is_a< ProjectDependency >(*i))
			continue;

		Ref< ui::ListViewItem > dependencyItem = new ui::ListViewItem();
		dependencyItem->setText(0, (*i)->getName());
		dependencyItem->setText(1, (*i)->getLocation());
		dependencyItem->setText(2, (*i)->getInheritIncludePaths() ? L"Yes" : L"No");
		dependencyItem->setText(3, c_link[(*i)->getLink()]);
		dependencyItem->setData(L"DEPENDENCY", *i);
		dependencyItems->add(dependencyItem);
	}

	m_listDependencies->setItems(dependencyItems);

	// Get available projects, remove all local projects which are already in dependency list.
	RefArray< Project > projects = m_solution->getProjects();
	for (RefArray< Dependency >::iterator i = dependencies.begin(); i != dependencies.end(); ++i)
	{
		if (!is_a< ProjectDependency >(*i))
			continue;

		RefArray< Project >::iterator j = std::find(projects.begin(), projects.end(), static_cast< ProjectDependency* >((*i).ptr())->getProject());
		if (j != projects.end())
			projects.erase(j);
	}

	// Update drop down with available projects.
	projects.sort(ProjectSortPredicate());
	m_dropAvailable->removeAll();
	for(RefArray< Project >::iterator i = projects.begin(); i != projects.end(); ++i)
	{
		if (*i != m_project)
			m_dropAvailable->add((*i)->getName());
	}
}

void ProjectPropertyPage::eventEnableClick(ui::ButtonClickEvent* event)
{
	m_project->setEnable(m_checkEnable->isChecked());
}

void ProjectPropertyPage::eventFocusSource(ui::FocusEvent* event)
{
	if (event->lostFocus())
		m_project->setSourcePath(m_editSourcePath->getText());
}

void ProjectPropertyPage::eventDependencyDoubleClick(ui::MouseDoubleClickEvent* event)
{
	ui::Point mousePosition = event->getPosition();

	Ref< ui::ListViewItem > selectedItem = m_listDependencies->getSelectedItem();
	if (!selectedItem)
		return;

	Ref< Dependency > dependency = selectedItem->getData< Dependency >(L"DEPENDENCY");
	if (!dependency)
		return;

	int32_t column = m_listDependencies->getColumnFromPosition(mousePosition.x);
	if (column < 0)
		return;

	if (column == 1)	// Location
	{
		Ref< ExternalDependency > selectedDependency = dynamic_type_cast< ExternalDependency* >(dependency);
		if (!selectedDependency)
			return;

		ui::custom::InputDialog::Field inputFields[] =
		{
			{ L"Location", selectedDependency->getSolutionFileName(), 0 }
		};

		ui::custom::InputDialog inputDialog;
		inputDialog.create(
			this,
			L"External dependency",
			L"Change location",
			inputFields,
			sizeof_array(inputFields)
		);
		if (inputDialog.showModal() == ui::DrOk)
		{
			selectedDependency->setSolutionFileName(inputFields[0].value);
			updateDependencyList();
		}
		inputDialog.destroy();
	}
	else if (column == 2)	// Inherit
	{
		bool inherit = !dependency->getInheritIncludePaths();
		dependency->setInheritIncludePaths(inherit);
		updateDependencyList();
	}
	else if (column == 3)	// Link
	{
		int32_t link = (dependency->getLink() + 1) % (Dependency::LnkForce + 1);
		dependency->setLink((Dependency::Link)link);
		updateDependencyList();
	}
}

void ProjectPropertyPage::eventClickAdd(ui::ButtonClickEvent* event)
{
	std::wstring dependencyName = m_dropAvailable->getSelectedItem();
	if (!dependencyName.empty())
	{
		const RefArray< Project >& projects = m_solution->getProjects();
		for(RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
		{
			if ((*i)->getName() == dependencyName)
			{
				Ref< ProjectDependency > dependency = new ProjectDependency(*i);
				m_project->addDependency(dependency);
				break;
			}
		}
		m_dropAvailable->select(-1);
		updateDependencyList();
	}
}

void ProjectPropertyPage::eventClickRemove(ui::ButtonClickEvent* event)
{
	Ref< ui::ListViewItem > selectedItem = m_listDependencies->getSelectedItem();
	if (!selectedItem)
		return;

	Ref< Dependency > selectedDependency = selectedItem->getData< Dependency >(L"DEPENDENCY");
	T_ASSERT (selectedDependency);

	RefArray< Dependency > dependencies = m_project->getDependencies();
	RefArray< Dependency >::iterator i = std::find(dependencies.begin(), dependencies.end(), selectedDependency);
	T_ASSERT (i != dependencies.end());

	dependencies.erase(i);
	m_project->setDependencies(dependencies);
	
	updateDependencyList();
}

void ProjectPropertyPage::eventClickAddExternal(ui::ButtonClickEvent* event)
{
	ui::FileDialog fileDialog;
	fileDialog.create(this, L"Select solution", L"SolutionBuilder solutions;*.xms");

	Path filePath;
	if (fileDialog.showModal(filePath))
	{
		Ref< Solution > solution = SolutionLoader().load(filePath.getPathName());
		if (solution)
		{
			ImportProjectDialog importDialog;
			importDialog.create(this, L"Select project(s)", false, solution);

			if (importDialog.showModal() == ui::DrOk)
			{
				RefArray< Project > externalProjects;
				importDialog.getSelectedProjects(externalProjects);

				for (RefArray< Project >::iterator i = externalProjects.begin(); i != externalProjects.end(); ++i)
					m_project->addDependency(new ExternalDependency(filePath.getPathName(), (*i)->getName()));

				updateDependencyList();
			}

			importDialog.destroy();
		}
		else
			ui::MessageBox::show(this, L"Unable to open solution", L"Error", ui::MbIconExclamation | ui::MbOk);
	}

	fileDialog.destroy();
}
