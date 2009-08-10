#include "Editor/App/ShortcutsSettingsPage.h"
#include "Editor/Settings.h"
#include "Ui/Application.h"
#include "Ui/MethodHandler.h"
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
	m_editShortcut->create(container, 0, 0, ui::WsClientBorder);
	m_editShortcut->addChangeEventHandler(ui::createMethodHandler(this, &ShortcutsSettingsPage::eventShortcutModified));

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

			m_gridShortcuts->addRow(row);
		}
	}

	parent->setText(i18n::Text(L"EDITOR_SETTINGS_SHORTCUTS"));
	return true;
}

void ShortcutsSettingsPage::destroy()
{
}

bool ShortcutsSettingsPage::apply(Settings* settings)
{
	return true;
}

void ShortcutsSettingsPage::eventShortcutSelect(ui::Event* event)
{
	RefArray< ui::custom::GridRow > selectedRows;
	m_gridShortcuts->getRows(selectedRows, ui::custom::GridView::GfSelectedOnly);

	if (selectedRows.size() == 1)
	{
		// @fixme
		m_editShortcut->setEnable(true);
	}
	else
	{
		m_editShortcut->set(0, 0);
		m_editShortcut->setEnable(false);
	}
}

void ShortcutsSettingsPage::eventShortcutModified(ui::Event* event)
{
	// @fixme
}

	}
}
