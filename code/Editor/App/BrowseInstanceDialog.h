/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Containers/ThreadsafeFifo.h"
#include "Core/Thread/Event.h"
#include "Ui/ConfigDialog.h"

namespace traktor
{

class PropertyGroup;
class Thread;

}

namespace traktor::db
{

class Database;
class Group;
class Instance;

}

namespace traktor::ui
{

class PreviewItem;
class PreviewList;
class PreviewSelectionChangeEvent;
class TreeView;
class TreeViewItem;

}

namespace traktor::editor
{

class IBrowseFilter;
class IBrowsePreview;
class IEditor;

class BrowseInstanceDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	explicit BrowseInstanceDialog(IEditor* editor, PropertyGroup* settings);

	bool create(ui::Widget* parent, db::Database* database, const IBrowseFilter* filter);

	virtual void destroy() override final;

	Ref< db::Instance > getInstance();

private:
	IEditor* m_editor;
	Ref< PropertyGroup > m_settings;
	Ref< ui::TreeView > m_treeDatabase;
	Ref< ui::PreviewList > m_listInstances;
	RefArray< const IBrowsePreview > m_browsePreview;
	Ref< db::Instance > m_instance;

	Thread* m_threadGeneratePreview;
	ThreadsafeFifo< std::function< void() > > m_previewTasks;
	Event m_previewTaskEvent;

	ui::TreeViewItem* buildGroupItems(ui::TreeView* treeView, ui::TreeViewItem* parent, db::Group* group, const IBrowseFilter* filter);

	void updatePreviewList();

	void eventTreeItemSelected(ui::SelectionChangeEvent* event);

	void eventListItemSelected(ui::PreviewSelectionChangeEvent* event);

	void eventListDoubleClick(ui::MouseDoubleClickEvent* event);

	void taskGeneratePreview(ui::PreviewItem* item);

	void threadGeneratePreview();
};

}
