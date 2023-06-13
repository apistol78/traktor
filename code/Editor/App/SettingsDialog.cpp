/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Editor/ISettingsPage.h"
#include "Editor/App/GeneralSettingsPage.h"
#include "Editor/App/SettingsDialog.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

struct SettingsPagePredicate
{
	bool operator () (const TypeInfo* lh, const TypeInfo* rh) const
	{
		// Ensure general settings page is always first.
		if (is_type_a(type_of< GeneralSettingsPage >(), *lh))
			return true;
		else if (is_type_a(type_of< GeneralSettingsPage >(), *rh))
			return false;

		// Sort using name of type.
		int32_t r = compareIgnoreCase(lh->getName(), rh->getName());
		if (r < 0)
			return true;
		else
			return false;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.SettingsDialog", SettingsDialog, ui::ConfigDialog)

bool SettingsDialog::create(ui::Widget* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container;

	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"EDITOR_SETTINGS_TITLE"),
		700_ut,
		600_ut,
		ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable,
		new ui::FloodLayout()
	))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	addEventHandler< ui::ButtonClickEvent >(this, &SettingsDialog::eventDialogClick);

	// Create page container.
	Ref< ui::Tab > tab = new ui::Tab();
	tab->create(this, ui::WsTabStop);

	// Create setting pages.
	std::vector< const TypeInfo* > types;
	for (const auto settingPageType : type_of< ISettingsPage >().findAllOf(false))
		types.push_back(settingPageType);
	std::sort(types.begin(), types.end(), SettingsPagePredicate());

	for (std::vector< const TypeInfo* >::const_iterator i = types.begin(); i != types.end(); ++i)
	{
		Ref< ISettingsPage > settingsPage = dynamic_type_cast< ISettingsPage* >((*i)->createInstance());
		if (!settingsPage)
			continue;

		Ref< ui::TabPage > tabPage = new ui::TabPage();
		if (!tabPage->create(tab, L"", new ui::FloodLayout(ui::Size(4, 4))))
			continue;

		if (!settingsPage->create(tabPage, originalSettings, settings, shortcutCommands))
			continue;

		m_settingPages.push_back(settingsPage);

		tab->addPage(tabPage);
	}

	if (tab->getPageCount() > 0)
		tab->setActivePage(tab->getPage(0));

	m_settings = settings;
	return true;
}

void SettingsDialog::destroy()
{
	for (auto settingPage : m_settingPages)
		settingPage->destroy();

	m_settingPages.resize(0);

	ui::ConfigDialog::destroy();
}

void SettingsDialog::eventDialogClick(ui::ButtonClickEvent* event)
{
	if ((ui::DialogResult)event->getCommand().getId() == ui::DialogResult::Ok)
	{
		for (auto settingPage : m_settingPages)
			settingPage->apply(m_settings);
	}
}

	}
}
