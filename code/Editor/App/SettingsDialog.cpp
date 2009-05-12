#include "Editor/App/SettingsDialog.h"
#include "Editor/Settings.h"
#include "Ui/Application.h"
#include "Ui/FloodLayout.h"
#include "Ui/TableLayout.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/CheckBox.h"
#include "Ui/Button.h"
#include "Ui/Static.h"
#include "Ui/Edit.h"
#include "Ui/ListBox.h"
#include "Ui/ListView.h"
#include "Ui/ListViewItems.h"
#include "Ui/ListViewItem.h"
#include "Ui/DropDown.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/InputDialog.h"
#include "I18N/Text.h"
#include "Render/RenderSystem.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.SettingsDialog", SettingsDialog, ui::ConfigDialog)

bool SettingsDialog::create(ui::Widget* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container;

	if (!ui::ConfigDialog::create(parent, i18n::Text(L"EDITOR_SETTINGS_TITLE"), 500, 400, ui::ConfigDialog::WsDefaultResizable, gc_new< ui::FloodLayout >()))
		return false;

	addClickEventHandler(ui::createMethodHandler(this, &SettingsDialog::eventDialogClick));

	// Create page container.
	Ref< ui::Tab > tab = gc_new< ui::Tab >();
	tab->create(this, ui::WsBorder);

	Ref< ui::TabPage > pageGeneral = gc_new< ui::TabPage >();
	pageGeneral->create(tab, i18n::Text(L"EDITOR_SETTINGS_GENERAL"), gc_new< ui::TableLayout >(L"100%", L"*", 4, 4));
	tab->addPage(pageGeneral);

	Ref< ui::TabPage > pageShortcuts = gc_new< ui::TabPage >();
	pageShortcuts->create(tab, i18n::Text(L"EDITOR_SETTINGS_SHORTCUTS"), gc_new< ui::TableLayout >(L"100%", L"100%", 4, 4));
	tab->addPage(pageShortcuts);

	Ref< ui::TabPage > pageModules = gc_new< ui::TabPage >();
	pageModules->create(tab, i18n::Text(L"EDITOR_SETTINGS_MODULES"), gc_new< ui::TableLayout >(L"100%", L"100%", 4, 4));
	tab->addPage(pageModules);

	tab->setActivePage(pageGeneral);

	// Create "General" page.
	container = gc_new< ui::Container >();
	container->create(pageGeneral, ui::WsNone, gc_new< ui::TableLayout >(L"*,100%", L"*", 0, 4));

	Ref< ui::Static > staticRenderer = gc_new< ui::Static >();
	staticRenderer->create(container, i18n::Text(L"EDITOR_SETTINGS_RENDERER"));

	m_dropRenderSystem = gc_new< ui::DropDown >();
	m_dropRenderSystem->create(container, L"");

	std::wstring renderSystemType = settings->getProperty< PropertyString >(L"Editor.RenderSystem");

	std::vector< const Type* > renderSystemTypes;
	type_of< render::RenderSystem >().findAllOf(renderSystemTypes);

	for (std::vector< const Type* >::const_iterator i = renderSystemTypes.begin(); i != renderSystemTypes.end(); ++i)
	{
		if (!(*i)->isInstantiable())
			continue;

		std::wstring name = (*i)->getName();

		int index = m_dropRenderSystem->add(name);
		if (name == renderSystemType)
			m_dropRenderSystem->select(index);
	}

	Ref< ui::Static > staticDictionary = gc_new< ui::Static >();
	staticDictionary->create(container, i18n::Text(L"EDITOR_SETTINGS_DICTIONARY"));

	m_editDictionary = gc_new< ui::Edit >();
	m_editDictionary->create(container, settings->getProperty< PropertyString >(L"Editor.Dictionary"));

	m_checkBuildAtStartup = gc_new< ui::CheckBox >();
	m_checkBuildAtStartup->create(pageGeneral, i18n::Text(L"EDITOR_SETTINGS_BUILD_AT_STARTUP"));
	m_checkBuildAtStartup->setChecked(settings->getProperty< PropertyBoolean >(L"Editor.BuildAtStartup"));

	m_checkBuildWhenModified = gc_new< ui::CheckBox >();
	m_checkBuildWhenModified->create(pageGeneral, i18n::Text(L"EDITOR_SETTINGS_BUILD_WHEN_MODIFIED"));
	m_checkBuildWhenModified->setChecked(settings->getProperty< PropertyBoolean >(L"Editor.BuildWhenModified"));

	// Create "Shortcuts" page.
	Ref< ui::Container > containerShortcuts = gc_new< ui::Container >();
	containerShortcuts->create(pageShortcuts, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"100%,*", 0, 4));

	Ref< ui::custom::GridView > gridShortcuts = gc_new< ui::custom::GridView >();
	gridShortcuts->create(containerShortcuts, ui::custom::GridView::WsColumnHeader | ui::WsClientBorder | ui::WsDoubleBuffer);
	gridShortcuts->addColumn(gc_new< ui::custom::GridColumn >(i18n::Text(L"EDITOR_SETTINGS_COMMAND"), 200));
	gridShortcuts->addColumn(gc_new< ui::custom::GridColumn >(i18n::Text(L"EDITOR_SETTINGS_SHORTCUT"), 200));

	const PropertyGroup* shortcutGroup = checked_type_cast< const PropertyGroup* >(settings->getProperty(L"Editor.Shortcuts"));
	if (shortcutGroup)
	{
		for (std::list< ui::Command >::const_iterator i = shortcutCommands.begin(); i != shortcutCommands.end(); ++i)
		{
			Ref< ui::custom::GridRow > row = gc_new< ui::custom::GridRow >();

			row->addItem(gc_new< ui::custom::GridItem >(
				i->getName()
			));

			std::pair< int, int > key = shortcutGroup->getProperty< PropertyKey >(i->getName());
			if (key.first || key.second)
			{
				std::wstring keyDesc = L"";

				if (key.first & ui::KsControl)
					keyDesc = L"Ctrl";
				if (key.first & ui::KsMenu)
					keyDesc = keyDesc.empty() ? L"Alt" : keyDesc + L"+Alt";
				if (key.first & ui::KsShift)
					keyDesc = keyDesc.empty() ? L"Shift" : keyDesc + L"+Shift";

				std::wstring keyName = ui::Application::getInstance().translateVirtualKey(key.second);
				keyDesc = keyDesc.empty() ? keyName : keyDesc + L", " + keyName;

				row->addItem(gc_new< ui::custom::GridItem >(
					keyDesc
				));
			}
			else
			{
				row->addItem(gc_new< ui::custom::GridItem >(
					i18n::Text(L"EDITOR_SETTINGS_SHORTCUT_NOT_ASSIGNED")
				));
			}

			gridShortcuts->addRow(row);
		}
	}

	// Create "Modules" page.
	Ref< ui::Container > containerModules = gc_new< ui::Container >();
	containerModules->create(pageModules, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"100%,*", 0, 4));

	m_listModules = gc_new< ui::ListBox >();
	m_listModules->create(containerModules, i18n::Text(L"EDITOR_SETTINGS_DEPENDENT_MODULES"), ui::ListBox::WsDefault | ui::ListBox::WsSort);

	Ref< ui::Container > containerModulesTools = gc_new< ui::Container >();
	containerModulesTools->create(containerModules, ui::WsNone, gc_new< ui::TableLayout >(L"*,*", L"100%", 0, 4));

	Ref< ui::Button > buttonAddModule = gc_new< ui::Button >();
	buttonAddModule->create(containerModulesTools, i18n::Text(L"EDITOR_SETTINGS_ADD_MODULE"));
	buttonAddModule->addClickEventHandler(ui::createMethodHandler(this, &SettingsDialog::eventButtonAddModuleClick));

	Ref< ui::Button > buttonRemoveModule = gc_new< ui::Button >();
	buttonRemoveModule->create(containerModulesTools, i18n::Text(L"EDITOR_SETTINGS_REMOVE_MODULE"));
	buttonRemoveModule->addClickEventHandler(ui::createMethodHandler(this, &SettingsDialog::eventButtonRemoveModuleClick));

	const std::vector< std::wstring >& modules = settings->getProperty< PropertyStringArray >(L"Editor.Modules");
	for (std::vector< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
		m_listModules->add(*i);

	m_settings = settings;
	return true;
}

void SettingsDialog::destroy()
{
	ui::ConfigDialog::destroy();
}

void SettingsDialog::eventDialogClick(ui::Event* event)
{
	if (checked_type_cast< ui::CommandEvent* >(event)->getCommand() == ui::DrOk)
	{
		// Update render system type.
		m_settings->setProperty< PropertyString >(L"Editor.RenderSystem", m_dropRenderSystem->getSelectedItem());

		// Update dictionary.
		m_settings->setProperty< PropertyString >(L"Editor.Dictionary", m_editDictionary->getText());

		// Update build.
		m_settings->setProperty< PropertyBoolean >(L"Editor.BuildAtStartup", m_checkBuildAtStartup->isChecked());
		m_settings->setProperty< PropertyBoolean >(L"Editor.BuildWhenModified", m_checkBuildWhenModified->isChecked());

		// Update modules.
		std::vector< std::wstring > modules;
		for (int i = 0; i < m_listModules->count(); ++i)
			modules.push_back(m_listModules->getItem(i));
		m_settings->setProperty< PropertyStringArray >(L"Editor.Modules", modules);
	}
}

void SettingsDialog::eventButtonAddModuleClick(ui::Event* event)
{
	ui::custom::InputDialog::Field fields[] =
	{
		{ i18n::Text(L"EDITOR_SETTINGS_ADD_DEPENDENT_NAME"), L"", 0 }
	};

	ui::custom::InputDialog inputDialog;
	inputDialog.create(
		this,
		i18n::Text(L"EDITOR_SETTINGS_ADD_DEPENDENT_TITLE"),
		i18n::Text(L"EDITOR_SETTINGS_ADD_DEPENDENT_MESSAGE"),
		fields,
		sizeof_array(fields)
	);
	if (inputDialog.showModal() == ui::DrOk && !fields[0].value.empty())
		m_listModules->add(fields[0].value);
	inputDialog.destroy();
}

void SettingsDialog::eventButtonRemoveModuleClick(ui::Event* event)
{
	int selectedItem = m_listModules->getSelected();
	if (selectedItem >= 0)
		m_listModules->remove(selectedItem);
}

	}
}
