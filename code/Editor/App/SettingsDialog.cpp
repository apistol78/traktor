#include "Editor/App/SettingsDialog.h"
#include "Editor/ISettingsPage.h"
#include "I18N/Text.h"
#include "Ui/FloodLayout.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.SettingsDialog", SettingsDialog, ui::ConfigDialog)

bool SettingsDialog::create(ui::Widget* parent, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container;

	if (!ui::ConfigDialog::create(parent, i18n::Text(L"EDITOR_SETTINGS_TITLE"), 700, 600, ui::ConfigDialog::WsDefaultResizable, new ui::FloodLayout()))
		return false;

	addClickEventHandler(ui::createMethodHandler(this, &SettingsDialog::eventDialogClick));

	// Create page container.
	Ref< ui::Tab > tab = new ui::Tab();
	tab->create(this, ui::WsBorder | ui::WsTabStop);

	// Create setting pages.
	TypeInfoSet settingPageTypes;
	type_of< ISettingsPage >().findAllOf(settingPageTypes, false);

	for (TypeInfoSet::const_iterator i = settingPageTypes.begin(); i != settingPageTypes.end(); ++i)
	{
		Ref< ISettingsPage > settingsPage = dynamic_type_cast< ISettingsPage* >((*i)->createInstance());
		if (!settingsPage)
			continue;

		Ref< ui::TabPage > tabPage = new ui::TabPage();
		if (!tabPage->create(tab, L"", new ui::FloodLayout(ui::Size(4, 4))))
			continue;

		if (!settingsPage->create(tabPage, settings, shortcutCommands))
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

void SettingsDialog::eventDialogClick(ui::Event* event)
{
	if (checked_type_cast< ui::CommandEvent* >(event)->getCommand() == ui::DrOk)
	{
		for (RefArray< ISettingsPage >::iterator i = m_settingPages.begin(); i != m_settingPages.end(); ++i)
			(*i)->apply(m_settings);
	}
}

	}
}
