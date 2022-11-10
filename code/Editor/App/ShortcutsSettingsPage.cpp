/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/StringOutputStream.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/App/Shortcut.h"
#include "Editor/App/ShortcutsSettingsPage.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Button.h"
#include "Ui/Command.h"
#include "Ui/TableLayout.h"
#include "Ui/Container.h"
#include "Ui/GridView/GridView.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/ShortcutEdit.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.ShortcutsSettingsPage", 0, ShortcutsSettingsPage, ISettingsPage)

bool ShortcutsSettingsPage::create(ui::Container* parent, const PropertyGroup* originalSettings, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0, ui::dpi96(4))))
		return false;

	m_gridShortcuts = new ui::GridView();
	m_gridShortcuts->create(container, ui::GridView::WsColumnHeader | ui::WsDoubleBuffer);
	m_gridShortcuts->addColumn(new ui::GridColumn(i18n::Text(L"EDITOR_SETTINGS_COMMAND"), ui::dpi96(200)));
	m_gridShortcuts->addColumn(new ui::GridColumn(i18n::Text(L"EDITOR_SETTINGS_SHORTCUT"), ui::dpi96(200)));
	m_gridShortcuts->addColumn(new ui::GridColumn(i18n::Text(L"EDITOR_SETTINGS_SHORTCUT_INUSE"), ui::dpi96(500)));
	m_gridShortcuts->setSortColumn(0, false, ui::GridView::SmLexical);
	m_gridShortcuts->addEventHandler< ui::SelectionChangeEvent >(this, &ShortcutsSettingsPage::eventShortcutSelect);

	Ref< ui::Container > containerEdit = new ui::Container();
	if (!containerEdit->create(container, ui::WsNone, new ui::TableLayout(L"100%,*", L"*", 0, ui::dpi96(4))))
		return false;

	m_editShortcut = new ui::ShortcutEdit();
	m_editShortcut->create(containerEdit, 0, ui::VkNull);
	m_editShortcut->addEventHandler< ui::ContentChangeEvent >(this, &ShortcutsSettingsPage::eventShortcutModified);

	m_buttonResetAll = new ui::Button();
	m_buttonResetAll->create(containerEdit, i18n::Text(L"EDITOR_SETTINGS_RESET_ALL_SHORTCUTS"));
	m_buttonResetAll->addEventHandler< ui::ButtonClickEvent >(this, &ShortcutsSettingsPage::eventResetAll);

	Ref< const PropertyGroup > shortcutGroup = checked_type_cast< const PropertyGroup* >(settings->getProperty(L"Editor.Shortcuts"));
	if (shortcutGroup)
	{
		for (const auto& shortcutCommand : shortcutCommands)
		{
			Ref< const PropertyString > constPropertyKey = dynamic_type_cast< const PropertyString* >(shortcutGroup->getProperty(shortcutCommand.getName()));
			Ref< PropertyString > propertyKey = new PropertyString(PropertyString::get(constPropertyKey));

			Ref< ui::GridRow > row = new ui::GridRow();
			row->add(new ui::GridItem(
				shortcutCommand.getName()
			));
			row->add(new ui::GridItem(
				i18n::Text(L"EDITOR_SETTINGS_SHORTCUT_NOT_ASSIGNED")
			));
			row->add(new ui::GridItem(L""));
			row->setData(L"PROPERTYKEY", propertyKey);
			m_gridShortcuts->addRow(row);
		}

		updateShortcutGrid();
	}

	parent->setText(i18n::Text(L"EDITOR_SETTINGS_SHORTCUTS"));

	m_originalSettings = originalSettings;
	return true;
}

void ShortcutsSettingsPage::destroy()
{
}

bool ShortcutsSettingsPage::apply(PropertyGroup* settings)
{
	for (auto row : m_gridShortcuts->getRows())
	{
		const RefArray< ui::GridItem >& items = row->get();
		T_ASSERT(items.size() == 3);

		Ref< PropertyString > propertyKey = row->getData< PropertyString >(L"PROPERTYKEY");
		T_ASSERT(propertyKey);

		PropertyString::value_type_t value = PropertyString::get(propertyKey);
		if (!value.empty())
			settings->setProperty(
				L"Editor.Shortcuts/" + items[0]->getText(),
				propertyKey
			);
	}
	return true;
}

void ShortcutsSettingsPage::updateShortcutGrid()
{
	for (auto row : m_gridShortcuts->getRows())
	{
		const RefArray< ui::GridItem >& items = row->get();
		T_ASSERT(items.size() == 3);

		Ref< PropertyString > propertyKey = row->getData< PropertyString >(L"PROPERTYKEY");
		T_ASSERT(propertyKey);

		std::pair< int, ui::VirtualKey > key = parseShortcut(PropertyString::get(propertyKey));
		if (key.first || key.second != ui::VkNull)
		{
			std::wstring keyDesc = L"";

			if (key.first & ui::KsCommand)
			{
#if defined(__APPLE__)
				keyDesc = L"\x2318";
#else
				keyDesc = L"Ctrl";
#endif
			}
			else if (key.first & ui::KsControl)
				keyDesc = keyDesc.empty() ? L"Ctrl" : keyDesc + L"+Ctrl";
			if (key.first & ui::KsMenu)
				keyDesc = keyDesc.empty() ? L"Alt" : keyDesc + L"+Alt";
			if (key.first & ui::KsShift)
				keyDesc = keyDesc.empty() ? L"Shift" : keyDesc + L"+Shift";

			std::wstring keyName = ui::Application::getInstance()->translateVirtualKey(key.second);
			keyDesc = keyDesc.empty() ? keyName : keyDesc + L", " + keyName;

			items[1]->setText(keyDesc);

			// Check if combination is used by any other shortcuts.
			StringOutputStream ss;
			for (auto duplicateRow : m_gridShortcuts->getRows())
			{
				if (duplicateRow == row)
					continue;

				const RefArray< ui::GridItem >& duplicateItems = duplicateRow->get();
				T_ASSERT(items.size() == 3);

				Ref< PropertyString > duplicatePropertyKey = duplicateRow->getData< PropertyString >(L"PROPERTYKEY");
				T_ASSERT(duplicatePropertyKey);

				if (PropertyString::get(propertyKey) == PropertyString::get(duplicatePropertyKey))
				{
					if (!ss.empty())
						ss << L", ";
					ss << duplicateItems[0]->getText();
				}
			}
			items[2]->setText(ss.str());			
		}
		else
		{
			items[1]->setText(i18n::Text(L"EDITOR_SETTINGS_SHORTCUT_NOT_ASSIGNED"));
			items[2]->setText(L"");
		}
	}
	m_gridShortcuts->update();
}

void ShortcutsSettingsPage::eventShortcutSelect(ui::SelectionChangeEvent* event)
{
	RefArray< ui::GridRow > selectedRows;
	m_gridShortcuts->getRows(selectedRows, ui::GridView::GfSelectedOnly);

	if (selectedRows.size() == 1)
	{
		Ref< PropertyString > propertyKey = selectedRows[0]->getData< PropertyString >(L"PROPERTYKEY");
		if (propertyKey)
		{
			std::pair< int, ui::VirtualKey > value = parseShortcut(PropertyString::get(propertyKey));
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

void ShortcutsSettingsPage::eventShortcutModified(ui::ContentChangeEvent* event)
{
	Ref< ui::GridRow > gridRow = m_editShortcut->getData< ui::GridRow >(L"GRIDROW");
	if (gridRow)
	{
		gridRow->setData(L"PROPERTYKEY", new PropertyString(describeShortcut(std::make_pair(
			m_editShortcut->getKeyState(),
			m_editShortcut->getVirtualKey()
		))));
		updateShortcutGrid();
	}
}

void ShortcutsSettingsPage::eventResetAll(ui::ButtonClickEvent* event)
{
	Ref< const PropertyGroup > shortcutGroup = checked_type_cast< const PropertyGroup* >(m_originalSettings->getProperty(L"Editor.Shortcuts"));
	if (shortcutGroup)
	{
		for (auto row : m_gridShortcuts->getRows())
		{
			const RefArray< ui::GridItem >& items = row->get();
			T_ASSERT(items.size() == 2);

			std::wstring value = m_originalSettings->getProperty< std::wstring >(L"Editor.Shortcuts/" + items[0]->getText());
			row->setData(L"PROPERTYKEY", new PropertyString(value));
		}
		updateShortcutGrid();
	}
}

	}
}
