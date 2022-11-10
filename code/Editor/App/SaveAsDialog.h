/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/ConfigDialog.h"

namespace traktor
{

class PropertyGroup;

	namespace db
	{

class Database;
class Group;

	}

	namespace ui
	{

class Edit;
class TreeView;
class TreeViewItem;

	}

	namespace editor
	{

class IEditor;

class SaveAsDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	SaveAsDialog(const IEditor* editor, PropertyGroup* settings);

	bool create(ui::Widget* parent, db::Database* database);

	virtual void destroy() override final;

	db::Group* getGroup() const;

	std::wstring getInstanceName() const;

private:
	const IEditor* m_editor;
	Ref< PropertyGroup > m_settings;
	Ref< ui::TreeView > m_treeDatabase;
	Ref< ui::Edit > m_editInstanceName;
	Ref< db::Group > m_group;

	ui::TreeViewItem* buildGroupItems(ui::TreeView* treeView, ui::TreeViewItem* parent, db::Group* group);

	void eventTreeItemSelected(ui::SelectionChangeEvent* event);
};

	}
}

