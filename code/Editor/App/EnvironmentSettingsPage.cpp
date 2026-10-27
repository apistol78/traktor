/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Editor/App/EnvironmentSettingsPage.h"

#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "I18N/Text.h"
#include "Ui/Button.h"
#include "Ui/Container.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridItemContentChangeEvent.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.EnvironmentSettingsPage", 0, EnvironmentSettingsPage, ISettingsPage)

bool EnvironmentSettingsPage::create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0_ut, 4_ut)))
		return false;

	m_gridVariables = new ui::GridView();
	m_gridVariables->create(container, ui::WsDoubleBuffer | ui::GridView::WsColumnHeader | ui::GridView::WsAutoEdit);
	m_gridVariables->addColumn(new ui::GridColumn(i18n::Text(L"EDITOR_SETTINGS_ENVIRONMENT_NAME"), 200_ut, true));
	m_gridVariables->addColumn(new ui::GridColumn(i18n::Text(L"EDITOR_SETTINGS_ENVIRONMENT_VALUE"), 400_ut, true));
	m_gridVariables->addEventHandler< ui::GridItemContentChangeEvent >(this, &EnvironmentSettingsPage::eventVariableChange);

	// Variables with empty value are removed.
	if (Ref< const PropertyGroup > variables = settings->getProperty< PropertyGroup >(L"Editor.Environment"))
	{
		for (const auto& it : variables->getValues())
		{
			if (!is_a< PropertyString >(it.second))
				continue;

			const std::wstring value = PropertyString::get(it.second);
			if (!value.empty())
				m_gridVariables->addRow({ it.first, value });
		}
	}

	Ref< ui::Container > containerTools = new ui::Container();
	containerTools->create(container, ui::WsNone, new ui::TableLayout(L"*,*", L"100%", 0_ut, 4_ut));

	Ref< ui::Button > buttonAdd = new ui::Button();
	buttonAdd->create(containerTools, i18n::Text(L"EDITOR_SETTINGS_ADD_VARIABLE"));
	buttonAdd->addEventHandler< ui::ButtonClickEvent >(this, &EnvironmentSettingsPage::eventAddVariable);

	Ref< ui::Button > buttonRemove = new ui::Button();
	buttonRemove->create(containerTools, i18n::Text(L"EDITOR_SETTINGS_REMOVE_VARIABLE"));
	buttonRemove->addEventHandler< ui::ButtonClickEvent >(this, &EnvironmentSettingsPage::eventRemoveVariable);

	parent->setText(i18n::Text(L"EDITOR_SETTINGS_ENVIRONMENT"));

	m_originalSettings = originalSettings;
	return true;
}

void EnvironmentSettingsPage::destroy()
{
}

bool EnvironmentSettingsPage::apply(PropertyGroup* settings)
{
	Ref< PropertyGroup > variables = new PropertyGroup();

	// User settings only store differences so removed original variables are kept with empty value.
	if (Ref< const PropertyGroup > originalVariables = m_originalSettings->getProperty< PropertyGroup >(L"Editor.Environment"))
		for (const auto& it : originalVariables->getValues())
			variables->setProperty< PropertyString >(it.first, L"");

	for (auto row : m_gridVariables->getRows())
	{
		const std::wstring name = row->get(0)->getText();
		if (!name.empty())
			variables->setProperty< PropertyString >(name, row->get(1)->getText());
	}

	settings->setProperty(L"Editor.Environment", variables);
	return true;
}

void EnvironmentSettingsPage::eventVariableChange(ui::GridItemContentChangeEvent* event)
{
	// Accept edit, grid is read when settings are applied.
	event->consume();
}

void EnvironmentSettingsPage::eventAddVariable(ui::ButtonClickEvent* event)
{
	m_gridVariables->addRow({ L"NEW_VARIABLE", L"" });
}

void EnvironmentSettingsPage::eventRemoveVariable(ui::ButtonClickEvent* event)
{
	if (ui::GridRow* row = m_gridVariables->getSelectedRow())
		m_gridVariables->removeRow(row);
}

}
