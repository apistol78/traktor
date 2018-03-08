/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
		int32_t r = compareIgnoreCase< std::wstring >(lh->getName(), rh->getName());
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

	if (!ui::ConfigDialog::create(parent, i18n::Text(L"EDITOR_SETTINGS_TITLE"), ui::scaleBySystemDPI(700), ui::scaleBySystemDPI(600), ui::ConfigDialog::WsDefaultResizable, new ui::FloodLayout()))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	addEventHandler< ui::ButtonClickEvent >(this, &SettingsDialog::eventDialogClick);

	// Create page container.
	Ref< ui::Tab > tab = new ui::Tab();
	tab->create(this, ui::WsTabStop);

	// Create setting pages.
	TypeInfoSet settingPageTypes;
	type_of< ISettingsPage >().findAllOf(settingPageTypes, false);

	std::vector< const TypeInfo* > types(settingPageTypes.begin(), settingPageTypes.end());
	std::sort(types.begin(), types.end(), SettingsPagePredicate());

	int32_t four = ui::scaleBySystemDPI(4);
	for (std::vector< const TypeInfo* >::const_iterator i = types.begin(); i != types.end(); ++i)
	{
		Ref< ISettingsPage > settingsPage = dynamic_type_cast< ISettingsPage* >((*i)->createInstance());
		if (!settingsPage)
			continue;

		Ref< ui::TabPage > tabPage = new ui::TabPage();
		if (!tabPage->create(tab, L"", new ui::FloodLayout(ui::Size(four, four))))
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
	for (RefArray< ISettingsPage >::iterator i = m_settingPages.begin(); i != m_settingPages.end(); ++i)
		(*i)->destroy();

	m_settingPages.resize(0);

	ui::ConfigDialog::destroy();
}

void SettingsDialog::eventDialogClick(ui::ButtonClickEvent* event)
{
	if (event->getCommand() == ui::DrOk)
	{
		for (RefArray< ISettingsPage >::iterator i = m_settingPages.begin(); i != m_settingPages.end(); ++i)
			(*i)->apply(m_settings);
	}
}

	}
}
