#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/Serialization/ISerializable.h"
#include "Editor/App/ModulesSettingsPage.h"
#include "I18N/Text.h"
#include "Ui/TableLayout.h"
#include "Ui/Container.h"
#include "Ui/Button.h"
#include "Ui/ListBox.h"
#include "Ui/Custom/InputDialog.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.ModulesSettingsPage", 0, ModulesSettingsPage, ISettingsPage)

bool ModulesSettingsPage::create(ui::Container* parent, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0, 4)))
		return false;

	m_listModules = new ui::ListBox();
	m_listModules->create(container, i18n::Text(L"EDITOR_SETTINGS_DEPENDENT_MODULES"), ui::ListBox::WsDefault | ui::ListBox::WsSort);

	Ref< ui::Container > containerModulesTools = new ui::Container();
	containerModulesTools->create(container, ui::WsNone, new ui::TableLayout(L"*,*", L"100%", 0, 4));

	Ref< ui::Button > buttonAddModule = new ui::Button();
	buttonAddModule->create(containerModulesTools, i18n::Text(L"EDITOR_SETTINGS_ADD_MODULE"));
	buttonAddModule->addEventHandler< ui::ButtonClickEvent >(this, &ModulesSettingsPage::eventButtonAddModuleClick);

	Ref< ui::Button > buttonRemoveModule = new ui::Button();
	buttonRemoveModule->create(containerModulesTools, i18n::Text(L"EDITOR_SETTINGS_REMOVE_MODULE"));
	buttonRemoveModule->addEventHandler< ui::ButtonClickEvent >(this, &ModulesSettingsPage::eventButtonRemoveModuleClick);

	const std::set< std::wstring >& modules = settings->getProperty< PropertyStringSet >(L"Editor.Modules");
	for (std::set< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
		m_listModules->add(*i);

	parent->setText(i18n::Text(L"EDITOR_SETTINGS_MODULES"));
	return true;
}

void ModulesSettingsPage::destroy()
{
}

bool ModulesSettingsPage::apply(PropertyGroup* settings)
{
	std::set< std::wstring > modules;
	for (int32_t i = 0; i < m_listModules->count(); ++i)
		modules.insert(m_listModules->getItem(i));
	settings->setProperty< PropertyStringSet >(L"Editor.Modules", modules);
	return true;
}

void ModulesSettingsPage::eventButtonAddModuleClick(ui::ButtonClickEvent* event)
{
	ui::custom::InputDialog::Field fields[] =
	{
		{ i18n::Text(L"EDITOR_SETTINGS_ADD_DEPENDENT_NAME"), L"", 0, 0 }
	};

	ui::custom::InputDialog inputDialog;
	inputDialog.create(
		m_listModules,
		i18n::Text(L"EDITOR_SETTINGS_ADD_DEPENDENT_TITLE"),
		i18n::Text(L"EDITOR_SETTINGS_ADD_DEPENDENT_MESSAGE"),
		fields,
		sizeof_array(fields)
	);
	if (inputDialog.showModal() == ui::DrOk && !fields[0].value.empty())
		m_listModules->add(fields[0].value);
	inputDialog.destroy();
}

void ModulesSettingsPage::eventButtonRemoveModuleClick(ui::ButtonClickEvent* event)
{
	int selectedItem = m_listModules->getSelected();
	if (selectedItem >= 0)
		m_listModules->remove(selectedItem);
}

	}
}
