#include "Core/Serialization/ISerializable.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Editor/App/Shortcut.h"
#include "Editor/App/ShortcutsSettingsPage.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Command.h"
#include "Ui/TableLayout.h"
#include "Ui/Container.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/ShortcutEdit.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.ShortcutsSettingsPage", 0, ShortcutsSettingsPage, ISettingsPage)

bool ShortcutsSettingsPage::create(ui::Container* parent, PropertyGroup* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 0, ui::scaleBySystemDPI(4))))
		return false;

	m_gridShortcuts = new ui::custom::GridView();
	m_gridShortcuts->create(container, ui::custom::GridView::WsColumnHeader | ui::WsDoubleBuffer);
	m_gridShortcuts->addColumn(new ui::custom::GridColumn(i18n::Text(L"EDITOR_SETTINGS_COMMAND"), ui::scaleBySystemDPI(200)));
	m_gridShortcuts->addColumn(new ui::custom::GridColumn(i18n::Text(L"EDITOR_SETTINGS_SHORTCUT"), ui::scaleBySystemDPI(200)));
	m_gridShortcuts->addEventHandler< ui::SelectionChangeEvent >(this, &ShortcutsSettingsPage::eventShortcutSelect);

	m_editShortcut = new ui::custom::ShortcutEdit();
	m_editShortcut->create(container, 0, ui::VkNull);
	m_editShortcut->addEventHandler< ui::ContentChangeEvent >(this, &ShortcutsSettingsPage::eventShortcutModified);

	Ref< const PropertyGroup > shortcutGroup = checked_type_cast< const PropertyGroup* >(settings->getProperty(L"Editor.Shortcuts"));
	if (shortcutGroup)
	{
		for (std::list< ui::Command >::const_iterator i = shortcutCommands.begin(); i != shortcutCommands.end(); ++i)
		{
			Ref< const PropertyString > constPropertyKey = dynamic_type_cast< const PropertyString* >(shortcutGroup->getProperty(i->getName()));
			Ref< PropertyString > propertyKey = new PropertyString(PropertyString::get(constPropertyKey));

			Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
			row->add(new ui::custom::GridItem(
				i->getName()
			));
			row->add(new ui::custom::GridItem(
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

bool ShortcutsSettingsPage::apply(PropertyGroup* settings)
{
	const RefArray< ui::custom::GridRow >& rows = m_gridShortcuts->getRows();
	for (RefArray< ui::custom::GridRow >::const_iterator i = rows.begin(); i != rows.end(); ++i)
	{
		const RefArray< ui::custom::GridCell >& items = (*i)->get();
		T_ASSERT (items.size() == 2);

		Ref< PropertyString > propertyKey = (*i)->getData< PropertyString >(L"PROPERTYKEY");
		T_ASSERT (propertyKey);

		PropertyString::value_type_t value = PropertyString::get(propertyKey);
		if (!value.empty())
			settings->setProperty(
				L"Editor.Shortcuts/" + checked_type_cast< ui::custom::GridItem*, false >(items[0])->getText(),
				propertyKey
			);
	}
	return true;
}

void ShortcutsSettingsPage::updateShortcutGrid()
{
	const RefArray< ui::custom::GridRow >& rows = m_gridShortcuts->getRows();
	for (RefArray< ui::custom::GridRow >::const_iterator i = rows.begin(); i != rows.end(); ++i)
	{
		const RefArray< ui::custom::GridCell >& items = (*i)->get();
		T_ASSERT (items.size() == 2);

		Ref< PropertyString > propertyKey = (*i)->getData< PropertyString >(L"PROPERTYKEY");
		T_ASSERT (propertyKey);

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

			checked_type_cast< ui::custom::GridItem*, false >(items[1])->setText(keyDesc);
		}
		else
			checked_type_cast< ui::custom::GridItem*, false >(items[1])->setText(i18n::Text(L"EDITOR_SETTINGS_SHORTCUT_NOT_ASSIGNED"));
	}
	m_gridShortcuts->update();
}

void ShortcutsSettingsPage::eventShortcutSelect(ui::SelectionChangeEvent* event)
{
	RefArray< ui::custom::GridRow > selectedRows;
	m_gridShortcuts->getRows(selectedRows, ui::custom::GridView::GfSelectedOnly);

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
	Ref< ui::custom::GridRow > gridRow = m_editShortcut->getData< ui::custom::GridRow >(L"GRIDROW");
	if (gridRow)
	{
		gridRow->setData(L"PROPERTYKEY", new PropertyString(describeShortcut(std::make_pair(
			m_editShortcut->getKeyState(),
			m_editShortcut->getVirtualKey()
		))));
		updateShortcutGrid();
	}
}

	}
}
