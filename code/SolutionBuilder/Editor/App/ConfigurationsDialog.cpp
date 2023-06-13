/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <Ui/Application.h>
#include <Ui/Button.h>
#include <Ui/Container.h>
#include <Ui/StyleBitmap.h>
#include <Ui/TableLayout.h>
#include <Ui/InputDialog.h>
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/Configuration.h"
#include "ConfigurationsDialog.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sb.ConfigurationDialog", ConfigurationsDialog, ui::ConfigDialog)

bool ConfigurationsDialog::create(ui::Widget* parent, Solution* solution)
{
	if (!ui::ConfigDialog::create(
		parent,
		L"Edit configurations",
		400_ut,
		300_ut,
		ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"100%,*", 4_ut, 4_ut)
	))
		return false;

	setIcon(new ui::StyleBitmap(L"SolutionBuilder.Icon"));

	m_listConfigurations = new ui::ListBox();
	m_listConfigurations->create(this, ui::ListBox::WsSingle);

	Ref< ui::Container > container = new ui::Container();
	container->create(this, ui::WsNone, new ui::TableLayout(L"*,*,*", L"*", 0_ut, 4_ut));

	Ref< ui::Button > buttonNew = new ui::Button();
	buttonNew->create(container, L"New...");
	buttonNew->addEventHandler< ui::ButtonClickEvent >(this, &ConfigurationsDialog::eventButtonNew);

	Ref< ui::Button > buttonRename = new ui::Button();
	buttonRename->create(container, L"Rename...");
	buttonRename->addEventHandler< ui::ButtonClickEvent >(this, &ConfigurationsDialog::eventButtonRename);

	Ref< ui::Button > buttonRemove = new ui::Button();
	buttonRemove->create(container, L"Remove");
	buttonRemove->addEventHandler< ui::ButtonClickEvent >(this, &ConfigurationsDialog::eventButtonRemove);

	SmallSet< std::wstring > configurations;

	// Get all unique configuration names.
	const RefArray< Project >& projects = solution->getProjects();
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const RefArray< Configuration >& projectConfigurations = (*i)->getConfigurations();
		for (RefArray< Configuration >::const_iterator j = projectConfigurations.begin(); j != projectConfigurations.end(); ++j)
			configurations.insert((*j)->getName());
	}

	// Populate configuration list.
	for (const auto& configuration : configurations)
		m_listConfigurations->add(configuration);

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
	T_ASSERT(!current.empty());

	ui::InputDialog::Field inputFields[] =
	{
		ui::InputDialog::Field(L"Name", current)
	};

	ui::InputDialog inputDialog;
	inputDialog.create(
		this,
		L"Enter name",
		L"Enter name of new configuration",
		inputFields,
		sizeof_array(inputFields)
	);
	if (inputDialog.showModal() == ui::DialogResult::Ok)
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
	T_ASSERT(!current.empty());

	ui::InputDialog::Field inputFields[] =
	{
		ui::InputDialog::Field(L"Name", current)
	};

	ui::InputDialog inputDialog;
	inputDialog.create(
		this,
		L"Rename",
		L"Enter new name of configuration \"" + current + L"\"",
		inputFields,
		sizeof_array(inputFields)
	);
	if (inputDialog.showModal() == ui::DialogResult::Ok)
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
	T_ASSERT(!name.empty());

	Action action = { AtRemove, name, L"" };
	m_actions.push_back(action);
	m_listConfigurations->remove(selectedId);
}

	}
}
