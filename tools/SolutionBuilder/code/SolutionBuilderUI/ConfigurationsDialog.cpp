#include <Ui/Application.h>
#include <Ui/TableLayout.h>
#include <Ui/Container.h>
#include <Ui/Button.h>
#include <Ui/Custom/InputDialog.h>
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/Configuration.h"
#include "ConfigurationsDialog.h"

using namespace traktor;

bool ConfigurationsDialog::create(ui::Widget* parent, Solution* solution)
{
	if (!ui::ConfigDialog::create(
		parent,
		L"Edit configurations",
		ui::scaleBySystemDPI(400),
		ui::scaleBySystemDPI(300),
		ui::ConfigDialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"100%,*", 4, 4)
	))
		return false;

	m_listConfigurations = new ui::ListBox();
	m_listConfigurations->create(this, L"", ui::WsClientBorder);

	Ref< ui::Container > container = new ui::Container();
	container->create(this, ui::WsNone, new ui::TableLayout(L"*,*,*", L"*", 0, 4));

	Ref< ui::Button > buttonNew = new ui::Button();
	buttonNew->create(container, L"New...");
	buttonNew->addEventHandler< ui::ButtonClickEvent >(this, &ConfigurationsDialog::eventButtonNew);

	Ref< ui::Button > buttonRename = new ui::Button();
	buttonRename->create(container, L"Rename...");
	buttonRename->addEventHandler< ui::ButtonClickEvent >(this, &ConfigurationsDialog::eventButtonRename);

	Ref< ui::Button > buttonRemove = new ui::Button();
	buttonRemove->create(container, L"Remove");
	buttonRemove->addEventHandler< ui::ButtonClickEvent >(this, &ConfigurationsDialog::eventButtonRemove);

	std::set< std::wstring > configurations;

	// Get all unique configuration names.
	const RefArray< Project >& projects = solution->getProjects();
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const RefArray< Configuration >& projectConfigurations = (*i)->getConfigurations();
		for (RefArray< Configuration >::const_iterator j = projectConfigurations.begin(); j != projectConfigurations.end(); ++j)
			configurations.insert((*j)->getName());
	}

	// Populate configuration list.
	for (std::set< std::wstring >::const_iterator i = configurations.begin(); i != configurations.end(); ++i)
		m_listConfigurations->add(*i);

	return true;
}

const std::vector< ConfigurationsDialog::Action >& ConfigurationsDialog::getActions() const
{
	return m_actions;
}

void ConfigurationsDialog::eventButtonNew(ui::ButtonClickEvent* event)
{
	int selectedId = m_listConfigurations->getSelected();
	if (selectedId < 0)
		return;

	std::wstring current = m_listConfigurations->getItem(selectedId);
	T_ASSERT (!current.empty());

	ui::custom::InputDialog::Field inputFields[] =
	{
		{ L"Name", current, 0 }
	};

	ui::custom::InputDialog inputDialog;
	inputDialog.create(
		this,
		L"Enter name",
		L"Enter name of new configuration",
		inputFields,
		sizeof_array(inputFields)
	);
	if (inputDialog.showModal() == ui::DrOk)
	{
		Action action = { AtNew, inputFields[0].value, current };
		if (!action.name.empty())
		{
			m_actions.push_back(action);
			m_listConfigurations->add(action.name);
		}
	}
	inputDialog.destroy();
}

void ConfigurationsDialog::eventButtonRename(ui::ButtonClickEvent* event)
{
	int selectedId = m_listConfigurations->getSelected();
	if (selectedId < 0)
		return;

	std::wstring current = m_listConfigurations->getItem(selectedId);
	T_ASSERT (!current.empty());

	ui::custom::InputDialog::Field inputFields[] =
	{
		{ L"Name", current, 0 }
	};

	ui::custom::InputDialog inputDialog;
	inputDialog.create(
		this,
		L"Rename",
		L"Enter new name of configuration \"" + current + L"\"", 
		inputFields,
		sizeof_array(inputFields)
	);
	if (inputDialog.showModal() == ui::DrOk)
	{
		Action action = { AtRename, inputFields[0].value, current };
		if (!action.name.empty() && action.name != action.current)
		{
			m_actions.push_back(action);
			m_listConfigurations->setItem(selectedId, action.name);
		}
	}
	inputDialog.destroy();
}

void ConfigurationsDialog::eventButtonRemove(ui::ButtonClickEvent* event)
{
	int selectedId = m_listConfigurations->getSelected();
	if (selectedId < 0)
		return;

	std::wstring name = m_listConfigurations->getItem(selectedId);
	T_ASSERT (!name.empty());

	Action action = { AtRemove, name, L"" };
	m_actions.push_back(action);
	m_listConfigurations->remove(selectedId);
}
