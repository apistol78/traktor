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
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Aggregation.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ProjectDependency.h"
#include "SolutionBuilder/ExternalDependency.h"
#include "SolutionBuilder/SolutionLoader.h"
#include "SolutionBuilder/Editor/App/AggregationPropertyPage.h"
#include "SolutionBuilder/Editor/App/ImportProjectDialog.h"

namespace traktor
{
	namespace sb
	{
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.AggregationPropertyPage", AggregationPropertyPage, ui::Container)

bool AggregationPropertyPage::create(ui::Widget* parent)
{
	const int32_t f = ui::scaleBySystemDPI(4);

	if (!ui::Container::create(
		parent,
		ui::WsNone,
		new ui::TableLayout(L"100%", L"*,100%", f, f)
	))
		return false;

	m_checkEnable = new ui::CheckBox();
	m_checkEnable->create(this, L"Include aggregation in build");
	m_checkEnable->addEventHandler< ui::ButtonClickEvent >(this, &AggregationPropertyPage::eventEnableClick);

	Ref< ui::Container > container = new ui::Container();
	container->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"100%,*", 0, f));

	Ref< ui::Static > staticDependencies = new ui::Static();
	staticDependencies->create(container, L"Dependencies");

	m_listDependencies = new ui::ListView();
	m_listDependencies->create(container, ui::WsClientBorder | ui::ListView::WsReport);
	m_listDependencies->addColumn(L"Dependency", ui::scaleBySystemDPI(130));
	m_listDependencies->addColumn(L"Location", ui::scaleBySystemDPI(270));
	m_listDependencies->addColumn(L"Link", ui::scaleBySystemDPI(50));
	m_listDependencies->addEventHandler< ui::MouseDoubleClickEvent >(this, &AggregationPropertyPage::eventDependencyDoubleClick);

	Ref< ui::Static > staticAvailable = new ui::Static();
	staticAvailable->create(container, L"Available");

	Ref< ui::Container > containerAvailable = new ui::Container();
	containerAvailable->create(container, ui::WsNone, new ui::TableLayout(L"100%,*,*,*", L"*", 0, f));

	m_dropAvailable = new ui::DropDown();
	m_dropAvailable->create(containerAvailable);

	Ref< ui::Button > buttonAdd = new ui::Button();
	buttonAdd->create(containerAvailable, L"Add");
	buttonAdd->addEventHandler< ui::ButtonClickEvent >(this, &AggregationPropertyPage::eventClickAdd);

	Ref< ui::Button > buttonRemove = new ui::Button();
	buttonRemove->create(containerAvailable, L"Remove");
	buttonRemove->addEventHandler< ui::ButtonClickEvent >(this, &AggregationPropertyPage::eventClickRemove);

	Ref< ui::Button > buttonAddExternal = new ui::Button();
	buttonAddExternal->create(containerAvailable, L"External...");
	buttonAddExternal->addEventHandler< ui::ButtonClickEvent >(this, &AggregationPropertyPage::eventClickAddExternal);

	return true;
}

void AggregationPropertyPage::set(Solution* solution, Aggregation* aggregation)
{
	m_solution = solution;
	m_aggregation = aggregation;

	m_checkEnable->setChecked(aggregation->getEnable());

	updateDependencyList();
}

void AggregationPropertyPage::updateDependencyList()
{
	RefArray< Dependency > dependencies = m_aggregation->getDependencies();
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
		dependencyItem->setText(2, c_link[(*i)->getLink()]);
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
		dependencyItem->setText(2, c_link[(*i)->getLink()]);
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
		m_dropAvailable->add((*i)->getName());
}

void AggregationPropertyPage::eventEnableClick(ui::ButtonClickEvent* event)
{
	m_aggregation->setEnable(m_checkEnable->isChecked());
}

void AggregationPropertyPage::eventDependencyDoubleClick(ui::MouseDoubleClickEvent* event)
{
	ui::Point mousePosition = event->getPosition();

	Ref< ui::ListViewItem > selectedItem = m_listDependencies->getSelectedItem();
	if (!selectedItem)
		return;

	Ref< Dependency > dependency = selectedItem->getData< Dependency >(L"DEPENDENCY");
	if (!dependency)
		return;

	// Check if user double clicked on "link" column.
	int32_t left = m_listDependencies->getColumnWidth(0) + m_listDependencies->getColumnWidth(1);
	if (mousePosition.x < left)
	{
		Ref< ExternalDependency > selectedDependency = dynamic_type_cast< ExternalDependency* >(dependency);
		if (!selectedDependency)
			return;

		ui::custom::InputDialog::Field inputFields[] =
		{
			ui::custom::InputDialog::Field(L"Location", selectedDependency->getSolutionFileName())
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
	else
	{
		int32_t link = (dependency->getLink() + 1) % (Dependency::LnkForce + 1);
		dependency->setLink((Dependency::Link)link);
		updateDependencyList();
	}
}

void AggregationPropertyPage::eventClickAdd(ui::ButtonClickEvent* event)
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
				m_aggregation->addDependency(dependency);
				break;
			}
		}
		m_dropAvailable->select(-1);
		updateDependencyList();
	}
}

void AggregationPropertyPage::eventClickRemove(ui::ButtonClickEvent* event)
{
	Ref< ui::ListViewItem > selectedItem = m_listDependencies->getSelectedItem();
	if (!selectedItem)
		return;

	Ref< Dependency > selectedDependency = selectedItem->getData< Dependency >(L"DEPENDENCY");
	T_ASSERT (selectedDependency);

	RefArray< Dependency > dependencies = m_aggregation->getDependencies();
	RefArray< Dependency >::iterator i = std::find(dependencies.begin(), dependencies.end(), selectedDependency);
	T_ASSERT (i != dependencies.end());

	dependencies.erase(i);
	m_aggregation->setDependencies(dependencies);
	
	updateDependencyList();
}

void AggregationPropertyPage::eventClickAddExternal(ui::ButtonClickEvent* event)
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
					m_aggregation->addDependency(new ExternalDependency(filePath.getPathName(), (*i)->getName()));

				updateDependencyList();
			}

			importDialog.destroy();
		}
		else
			ui::MessageBox::show(this, L"Unable to open solution", L"Error", ui::MbIconExclamation | ui::MbOk);
	}

	fileDialog.destroy();
}

	}
}
