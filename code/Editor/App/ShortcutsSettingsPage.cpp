#include "Editor/App/ShortcutsSettingsPage.h"
#include "Editor/Settings.h"
#include "Ui/Application.h"
#include "Ui/MethodHandler.h"
#include "Ui/Event.h"
#include "Ui/Command.h"
#include "Ui/TableLayout.h"
#include "Ui/Container.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/ShortcutEdit.h"
#include "I18N/Text.h"
#include "Core/Serialization/Serializable.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.editor.ShortcutsSettingsPage", ShortcutsSettingsPage, ISettingsPage)

bool ShortcutsSettingsPage::create(ui::Container* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = gc_new< ui::Container >();
	if (!container->create(parent, ui::WsNone, gc_new< ui::TableLayout >(L"100%", L"100%,*", 0, 4)))
		return false;

	m_gridShortcuts = gc_new< ui::custom::GridView >();
	m_gridShortcuts->create(container, ui::custom::GridView::WsColumnHeader | ui::WsClientBorder | ui::WsDoubleBuffer);
	m_gridShortcuts->addColumn(gc_new< ui::custom::GridColumn >(i18n::Text(L"EDITOR_SETTINGS_COMMAND"), 200));
	m_gridShortcuts->addColumn(gc_new< ui::custom::GridColumn >(i18n::Text(L"EDITOR_SETTINGS_SHORTCUT"), 200));
	m_gridShortcuts->addSelectEventHandler(ui::createMethodHandler(this, &ShortcutsSettingsPage::eventShortcutSelect));

	m_editShortcut = gc_new< ui::custom::ShortcutEdit >();
	m_editShortcut->create(container, 0, ui::VkNull, ui::WsClientBorder);
	m_editShortcut->addChangeEventHandler(ui::createMethodHandler(this, &ShortcutsSettingsPage::eventShortcutModified));

	Ref< PropertyGroup > shortcutGroup = checked_type_cast< PropertyGroup* >(settings->getProperty(L"Editor.Shortcuts"));
	if (shortcutGroup)
	{
		for (std::list< ui::Command >::const_iterator i = shortcutCommands.begin(); i != shortcutCommands.end(); ++i)
		{
			Ref< PropertyKey > propertyKey = dynamic_type_cast< PropertyKey* >(shortcutGroup->getProperty(i->getName()));

			// Create a copy of property key as we don't want to modify settings without being applied.
			propertyKey = gc_new< PropertyKey >(cref(PropertyKey::get(propertyKey)));

			Ref< ui::custom::GridRow > row = gc_new< ui::custom::GridRow >();
			row->addItem(gc_new< ui::custom::GridItem >(
				i->getName()
			));
			row->addItem(gc_new< ui::custom::GridItem >(
				i18n::Text(L"EDITOR_SETTINGS_SHORTCUT_NOT_ASSIGNED")
			));
			row->setData(L"PROPERTYKEY", propertyKey);
			m_gridShortcuts->addRow(row);
		}

		updateShortcutGrid();
	}

	parent->setText(i18n::Text(L"EDITOR_SETTINGS_SHORTCUTS"));
	return true;
}

void ShortcutsSettingsPage::destroy()
{
}

bool ShortcutsSettingsPage::apply(Settings* settings)
{
	Ref< PropertyGroup > shortcutGroup = checked_type_cast< PropertyGroup* >(settings->getProperty(L"Editor.Shortcuts"));
	if (shortcutGroup)
		shortcutGroup = gc_new< PropertyGroup >(cref(PropertyGroup::get(shortcutGroup)));
	else
		shortcutGroup = gc_new< PropertyGroup >();

	const RefArray< ui::custom::GridRow >& rows = m_gridShortcuts->getRows();
	for (RefArray< ui::custom::GridRow >::const_iterator i = rows.begin(); i != rows.end(); ++i)
	{
		const RefArray< ui::custom::GridItem >& items = (*i)->getItems();
		T_ASSERT (items.size() == 2);

		Ref< PropertyKey > propertyKey = (*i)->getData< PropertyKey >(L"PROPERTYKEY");
		T_ASSERT (propertyKey);

		PropertyKey::value_type_t value = PropertyKey::get(propertyKey);
		if (value.first != 0 || value.second != 0)
			shortcutGroup->setProperty(
				items[0]->getText(),
				propertyKey
			);
	}

	settings->setProperty(L"Editor.Shortcuts", shortcutGroup);
	return true;
}

void ShortcutsSettingsPage::updateShortcutGrid()
{
	const RefArray< ui::custom::GridRow >& rows = m_gridShortcuts->getRows();
	for (RefArray< ui::custom::GridRow >::const_iterator i = rows.begin(); i != rows.end(); ++i)
	{
		const RefArray< ui::custom::GridItem >& items = (*i)->getItems();
		T_ASSERT (items.size() == 2);

		Ref< PropertyKey > propertyKey = (*i)->getData< PropertyKey >(L"PROPERTYKEY");
		T_ASSERT (propertyKey);

		std::pair< int, ui::VirtualKey > key = PropertyKey::get(propertyKey);
		if (key.first || key.second != ui::VkNull)
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

			items[1]->setText(keyDesc);
		}
		else
			items[1]->setText(i18n::Text(L"EDITOR_SETTINGS_SHORTCUT_NOT_ASSIGNED"));
	}
	m_gridShortcuts->update();
}

void ShortcutsSettingsPage::eventShortcutSelect(ui::Event* event)
{
	RefArray< ui::custom::GridRow > selectedRows;
	m_gridShortcuts->getRows(selectedRows, ui::custom::GridView::GfSelectedOnly);

	if (selectedRows.size() == 1)
	{
		Ref< PropertyKey > propertyKey = selectedRows[0]->getData< PropertyKey >(L"PROPERTYKEY");
		if (propertyKey)
		{
			std::pair< int, ui::VirtualKey > value = PropertyKey::get(propertyKey);
			m_editShortcut->set(value.first, value.second);
		}
		else
			m_editShortcut->set(0, ui::VkNull);
		m_editShortcut->setEnable(true);
		m_editShortcut->setData(L"GRIDROW", selectedRows[0]);
	}
	else
	{
		m_editShortcut->set(0, ui::VkNull);
		m_editShortcut->setEnable(false);
		m_editShortcut->setData(L"GRIDROW", 0);
	}

	m_editShortcut->setFocus();
	m_editShortcut->update();
}

void ShortcutsSettingsPage::eventShortcutModified(ui::Event* event)
{
	Ref< ui::custom::GridRow > gridRow = m_editShortcut->getData< ui::custom::GridRow >(L"GRIDROW");
	if (gridRow)
	{
		gridRow->setData(L"PROPERTYKEY", gc_new< PropertyKey >(cref(std::make_pair(
			m_editShortcut->getKeyState(),
			m_editShortcut->getVirtualKey()
		))));
		updateShortcutGrid();
	}
}

	}
}
