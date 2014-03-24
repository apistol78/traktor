#include <Ui/TableLayout.h>
#include <Ui/Static.h>
#include <Ui/Button.h>
#include <Ui/ListViewItem.h>
#include <Ui/FileDialog.h>
#include <Ui/MessageBox.h>
#include <Ui/Custom/InputDialog.h>
#include <Ui/MethodHandler.h>
#include <Ui/Events/FocusEvent.h>
#include <Ui/Events/MouseEvent.h>
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Aggregation.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/SolutionLoader.h"
#include "SolutionBuilderUI/AggregationPropertyPage.h"
#include "SolutionBuilderUI/ImportProjectDialog.h"
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

bool AggregationPropertyPage::create(ui::Widget* parent)
{
	if (!ui::Container::create(
		parent,
		ui::WsNone,
		new ui::TableLayout(L"100%", L"*,100%", 4, 4)
	))
		return false;

	m_checkEnable = new ui::CheckBox();
	m_checkEnable->create(this, L"Include aggregation in build");
	m_checkEnable->addClickEventHandler(ui::createMethodHandler(this, &AggregationPropertyPage::eventEnableClick));

	Ref< ui::Container > container = new ui::Container();
	container->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"100%,*", 0, 4));

	Ref< ui::Static > staticDependencies = new ui::Static();
	staticDependencies->create(container, L"Dependencies");

	m_listDependencies = new ui::ListView();
	m_listDependencies->create(container, ui::WsClientBorder | ui::ListView::WsReport);
	m_listDependencies->addColumn(L"Dependency", 130);
	m_listDependencies->addColumn(L"Location", 270);
	m_listDependencies->addColumn(L"Link", 50);
	m_listDependencies->addDoubleClickEventHandler(ui::createMethodHandler(this, &AggregationPropertyPage::eventDependencyDoubleClick));

	Ref< ui::Static > staticAvailable = new ui::Static();
	staticAvailable->create(container, L"Available");

	Ref< ui::Container > containerAvailable = new ui::Container();
	containerAvailable->create(container, ui::WsNone, new ui::TableLayout(L"100%,*,*,*", L"*", 0, 4));

	m_dropAvailable = new ui::DropDown();
	m_dropAvailable->create(containerAvailable);

	Ref< ui::Button > buttonAdd = new ui::Button();
	buttonAdd->create(containerAvailable, L"Add");
	buttonAdd->addClickEventHandler(ui::createMethodHandler(
		this,
		&AggregationPropertyPage::eventClickAdd
	));

	Ref< ui::Button > buttonRemove = new ui::Button();
	buttonRemove->create(containerAvailable, L"Remove");
	buttonRemove->addClickEventHandler(ui::createMethodHandler(
		this,
		&AggregationPropertyPage::eventClickRemove
	));

	Ref< ui::Button > buttonAddExternal = new ui::Button();
	buttonAddExternal->create(containerAvailable, L"External...");
	buttonAddExternal->addClickEventHandler(ui::createMethodHandler(
		this,
		&AggregationPropertyPage::eventClickAddExternal
	));

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

void AggregationPropertyPage::eventEnableClick(ui::Event* event)
{
	m_aggregation->setEnable(m_checkEnable->isChecked());
}

void AggregationPropertyPage::eventDependencyDoubleClick(ui::Event* event)
{
	ui::Point mousePosition = checked_type_cast< const ui::MouseEvent* >(event)->getPosition();

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
	else
	{
		int32_t link = (dependency->getLink() + 1) % (Dependency::LnkForce + 1);
		dependency->setLink((Dependency::Link)link);
		updateDependencyList();
	}
}

void AggregationPropertyPage::eventClickAdd(ui::Event* event)
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

void AggregationPropertyPage::eventClickRemove(ui::Event* event)
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

void AggregationPropertyPage::eventClickAddExternal(ui::Event* event)
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
			importDialog.create(this, L"Select project(s)", solution);

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
