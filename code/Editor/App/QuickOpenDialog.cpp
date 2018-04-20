/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IEditor.h"
#include "Editor/App/QuickOpenDialog.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.QuickOpenDialog", QuickOpenDialog, ui::Dialog)

QuickOpenDialog::QuickOpenDialog(IEditor* editor)
:	m_editor(editor)
{
}

bool QuickOpenDialog::create(ui::Widget* parent)
{
	if (!ui::Dialog::create(parent, i18n::Text(L"EDITOR_QUICK_OPEN_TITLE"), ui::dpi96(600), ui::dpi96(250), ui::Dialog::WsDefaultResizable, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	m_editFilter = new ui::Edit();
	m_editFilter->create(this, L"", ui::WsClientBorder | ui::WsWantAllInput);
	m_editFilter->addEventHandler< ui::ContentChangeEvent >(this, &QuickOpenDialog::eventFilterChange);
	m_editFilter->addEventHandler< ui::KeyDownEvent >(this, &QuickOpenDialog::eventFilterKey);

	m_gridSuggestions = new ui::custom::GridView();
	m_gridSuggestions->create(this, ui::WsDoubleBuffer);
	m_gridSuggestions->addColumn(new ui::custom::GridColumn(i18n::Text(L"EDITOR_QUICK_OPEN_COLUMN_NAME"), ui::dpi96(180)));
	m_gridSuggestions->addColumn(new ui::custom::GridColumn(i18n::Text(L"EDITOR_QUICK_OPEN_COLUMN_PATH"), ui::dpi96(400)));
	m_gridSuggestions->addEventHandler< ui::SelectionChangeEvent >(this, &QuickOpenDialog::eventSuggestionSelect);

	db::recursiveFindChildInstances(
		m_editor->getSourceDatabase()->getRootGroup(),
		db::FindInstanceAll(),
		m_instances
	);

	update();
	return true;
}

db::Instance* QuickOpenDialog::showDialog()
{
	m_editFilter->setText(L"");
	m_editFilter->setFocus();

	updateSuggestions(L"");

	if (showModal() != ui::DrOk || m_gridSuggestions->getSelectedRow() == 0)
		return 0;

	return m_gridSuggestions->getSelectedRow()->getData< db::Instance >(L"INSTANCE");
}

void QuickOpenDialog::updateSuggestions(const std::wstring& filter)
{
	m_gridSuggestions->removeAllRows();

	for (RefArray< db::Instance >::const_iterator i = m_instances.begin(); i != m_instances.end(); ++i)
	{
		std::wstring instanceName = (*i)->getName();
		if (startsWith(toLower(instanceName), toLower(filter)))
		{
			Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
			row->add(new ui::custom::GridItem(instanceName));
			row->add(new ui::custom::GridItem((*i)->getPath()));
			row->setData(L"INSTANCE", *i);
			m_gridSuggestions->addRow(row);
		}
	}

	if (!filter.empty())
	{
		ui::custom::GridRow* row = m_gridSuggestions->getRow(0);
		if (row)
			row->setState(ui::custom::GridRow::RsSelected);
	}
}

void QuickOpenDialog::eventFilterChange(ui::ContentChangeEvent* event)
{
	// Check isModel in to make sure we haven't called endModal in key down event handler already.
	if (isModal())
	{
		std::wstring filter = m_editFilter->getText();
		updateSuggestions(filter);
	}
}

void QuickOpenDialog::eventFilterKey(ui::KeyDownEvent* event)
{
	if (event->getVirtualKey() == ui::VkReturn || event->getVirtualKey() == ui::VkSpace)
	{
		endModal(ui::DrOk);
		event->consume();
	}
	else if (event->getVirtualKey() == ui::VkEscape)
	{
		endModal(ui::DrCancel);
		event->consume();
	}
	else if (event->getVirtualKey() == ui::VkUp)
	{
		ui::custom::GridRow* row = m_gridSuggestions->getSelectedRow();
		if (row)
		{
			const RefArray< ui::custom::GridRow >& rows = m_gridSuggestions->getRows();
			RefArray< ui::custom::GridRow >::const_iterator i = std::find(rows.begin(), rows.end(), row);
			if (i != rows.end())
			{
				if (i != rows.begin())
					--i;

				m_gridSuggestions->deselectAll();
				(*i)->setState(ui::custom::GridRow::RsSelected);
			}
			else if (!rows.empty())
			{
				m_gridSuggestions->deselectAll();
				rows[0]->setState(ui::custom::GridRow::RsSelected);
			}
		}
		event->consume();
	}
	else if (event->getVirtualKey() == ui::VkDown)
	{
		ui::custom::GridRow* row = m_gridSuggestions->getSelectedRow();
		if (row)
		{
			const RefArray< ui::custom::GridRow >& rows = m_gridSuggestions->getRows();
			RefArray< ui::custom::GridRow >::const_iterator i = std::find(rows.begin(), rows.end(), row);
			if (i != rows.end())
			{
				if (++i != rows.end())
				{
					m_gridSuggestions->deselectAll();
					(*i)->setState(ui::custom::GridRow::RsSelected);
				}
			}
			else if (!rows.empty())
			{
				m_gridSuggestions->deselectAll();
				rows[0]->setState(ui::custom::GridRow::RsSelected);
			}
		}
		event->consume();
	}
}

void QuickOpenDialog::eventSuggestionSelect(ui::SelectionChangeEvent* event)
{
	if (m_gridSuggestions->getSelectedRow() != 0)
		endModal(ui::DrOk);
}

	}
}
