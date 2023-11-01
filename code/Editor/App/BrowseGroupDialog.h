/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
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

}

namespace traktor::db
{

class Database;
class Group;

}

namespace traktor::ui
{

class TreeView;
class TreeViewItem;

}

namespace traktor::editor
{

class IEditor;

class BrowseGroupDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	explicit BrowseGroupDialog(const IEditor* editor, PropertyGroup* settings);

	bool create(ui::Widget* parent, db::Database* database);

	virtual void destroy() override final;

	Ref< db::Group > getGroup();

private:
	const IEditor* m_editor;
	Ref< PropertyGroup > m_settings;
	Ref< ui::TreeView > m_treeDatabase;
	Ref< db::Group > m_group;

	ui::TreeViewItem* buildGroupItems(ui::TreeView* treeView, ui::TreeViewItem* parent, db::Group* group);

	void eventTreeItemSelected(ui::SelectionChangeEvent* event);
};

}

