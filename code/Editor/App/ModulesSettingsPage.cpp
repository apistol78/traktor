/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/Serialization/ISerializable.h"
#include "Editor/App/ModulesSettingsPage.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/Container.h"
#include "Ui/TableLayout.h"
#include "Ui/InputDialog.h"
#include "Ui/ListBox/ListBox.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.ModulesSettingsPage", 0, ModulesSettingsPage, ISettingsPage)

bool ModulesSettingsPage::create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0_ut, 4_ut)))
		return false;

	m_listModules = new ui::ListBox();
	m_listModules->create(container, ui::ListBox::WsSingle | ui::ListBox::WsSort);

	Ref< ui::Container > containerModulesTools = new ui::Container();
	containerModulesTools->create(container, ui::WsNone, new ui::TableLayout(L"*,*,*", L"100%", 0_ut, 4_ut));

	Ref< ui::Button > buttonAddModule = new ui::Button();
	buttonAddModule->create(containerModulesTools, i18n::Text(L"EDITOR_SETTINGS_ADD_MODULE"));
	buttonAddModule->addEventHandler< ui::ButtonClickEvent >(this, &ModulesSettingsPage::eventAddModule);

	Ref< ui::Button > buttonRemoveModule = new ui::Button();
	buttonRemoveModule->create(containerModulesTools, i18n::Text(L"EDITOR_SETTINGS_REMOVE_MODULE"));
	buttonRemoveModule->addEventHandler< ui::ButtonClickEvent >(this, &ModulesSettingsPage::eventRemoveModule);

	Ref< ui::Button > buttonResetModules = new ui::Button();
	buttonResetModules->create(containerModulesTools, i18n::Text(L"EDITOR_SETTINGS_RESET_MODULES"));
	buttonResetModules->addEventHandler< ui::ButtonClickEvent >(this, &ModulesSettingsPage::eventResetModules);

	auto modules = settings->getProperty< SmallSet< std::wstring > >(L"Editor.Modules");
	for (const auto& module : modules)
		m_listModules->add(module);

	parent->setText(i18n::Text(L"EDITOR_SETTINGS_MODULES"));

	m_originalSettings = originalSettings;
	return true;
}

void ModulesSettingsPage::destroy()
{
}

bool ModulesSettingsPage::apply(PropertyGroup* settings)
{
	SmallSet< std::wstring > modules;
	for (int32_t i = 0; i < m_listModules->count(); ++i)
		modules.insert(m_listModules->getItem(i));
	settings->setProperty< PropertyStringSet >(L"Editor.Modules", modules);
	return true;
}

void ModulesSettingsPage::eventAddModule(ui::ButtonClickEvent* event)
{
	ui::InputDialog::Field fields[] =
	{
		ui::InputDialog::Field(i18n::Text(L"EDITOR_SETTINGS_ADD_DEPENDENT_NAME"))
	};

	ui::InputDialog inputDialog;
	inputDialog.create(
		m_listModules,
		i18n::Text(L"EDITOR_SETTINGS_ADD_DEPENDENT_TITLE"),
		i18n::Text(L"EDITOR_SETTINGS_ADD_DEPENDENT_MESSAGE"),
		fields,
		sizeof_array(fields)
	);
	if (inputDialog.showModal() == ui::DialogResult::Ok && !fields[0].value.empty())
		m_listModules->add(fields[0].value);
	inputDialog.destroy();
}

void ModulesSettingsPage::eventRemoveModule(ui::ButtonClickEvent* event)
{
	int selectedItem = m_listModules->getSelected();
	if (selectedItem >= 0)
		m_listModules->remove(selectedItem);
}

void ModulesSettingsPage::eventResetModules(ui::ButtonClickEvent* event)
{
	m_listModules->removeAll();
	for (const auto& module : m_originalSettings->getProperty< SmallSet< std::wstring > >(L"Editor.Modules"))
		m_listModules->add(module);
}

	}
}
