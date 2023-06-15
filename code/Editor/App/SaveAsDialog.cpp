/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyObject.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Editor/App/SaveAsDialog.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/HierarchicalState.h"
#include "Ui/Static.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/MiniButton.h"
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewItem.h"

// Resources
#include "Resources/Folders.h"

#pragma warning(disable: 4344)

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.SaveAsDialog", SaveAsDialog, ui::ConfigDialog)

SaveAsDialog::SaveAsDialog(const IEditor* editor, PropertyGroup* settings)
:	m_editor(editor)
,	m_settings(settings)
{
}

bool SaveAsDialog::create(ui::Widget* parent, db::Database* database)
{
	if (!ui::ConfigDialog::create(
		parent,
		i18n::Text(L"SAVE_AS_TITLE"),
		400_ut,
		500_ut,
		ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable,
		new ui::TableLayout(L"100%", L"100%,*", 4_ut, 4_ut)
	))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	m_treeDatabase = new ui::TreeView();
	if (!m_treeDatabase->create(this, ui::WsDoubleBuffer | ui::WsTabStop))
		return false;
	m_treeDatabase->addImage(new ui::StyleBitmap(L"Editor.Database.Folders", 0));
	m_treeDatabase->addImage(new ui::StyleBitmap(L"Editor.Database.Folders", 1));
	m_treeDatabase->addEventHandler< ui::SelectionChangeEvent >(this, &SaveAsDialog::eventTreeItemSelected);

	m_editInstanceName = new ui::Edit();
	if (!m_editInstanceName->create(this, L"", ui::WsDoubleBuffer))
		return false;

	// Traverse database and filter out items.
	ui::TreeViewItem* groupRoot = buildGroupItems(
		m_treeDatabase,
		nullptr,
		database->getRootGroup()
	);

	if (groupRoot)
		groupRoot->sort(true);

	// Restore last state.
	Ref< ui::HierarchicalState > state = dynamic_type_cast< ui::HierarchicalState* >(m_settings->getProperty< Ref< ISerializable > >(L"Editor.SaveAsTreeState"));
	if (state)
		m_treeDatabase->applyState(state);

	m_treeDatabase->deselectAll();
	return true;
}

void SaveAsDialog::destroy()
{
	if (m_settings)
	{
		Ref< ui::HierarchicalState > state = m_treeDatabase->captureState();
		m_settings->setProperty< PropertyObject >(L"Editor.SaveAsTreeState", state);
	}
	ui::ConfigDialog::destroy();
}

db::Group* SaveAsDialog::getGroup() const
{
	return m_group;
}

std::wstring SaveAsDialog::getInstanceName() const
{
	return m_editInstanceName->getText();
}

ui::TreeViewItem* SaveAsDialog::buildGroupItems(ui::TreeView* treeView, ui::TreeViewItem* parent, db::Group* group)
{
	RefArray< db::Group > childGroups;
	group->getChildGroups(childGroups);

	Ref< ui::TreeViewItem > groupItem = treeView->createItem(parent, group->getName(), 1);
	groupItem->setImage(0, 0, 1);
	groupItem->setData(L"GROUP", group);

	for (auto childGroup : childGroups)
		buildGroupItems(treeView, groupItem, childGroup);

	return groupItem;
}

void SaveAsDialog::eventTreeItemSelected(ui::SelectionChangeEvent* event)
{
	RefArray< ui::TreeViewItem > items;
	if (m_treeDatabase->getItems(items, ui::TreeView::GfDescendants | ui::TreeView::GfSelectedOnly) == 1)
		m_group = items.back()->getData< db::Group >(L"GROUP");
	else
		m_group = nullptr;
}

	}
}
