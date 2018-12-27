/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_BrowseInstanceDialog_H
#define traktor_editor_BrowseInstanceDialog_H

#include "Core/Containers/ThreadsafeFifo.h"
#include "Core/Thread/Event.h"
#include "Ui/ConfigDialog.h"

namespace traktor
{

class Functor;
class PropertyGroup;
class Thread;

	namespace db
	{

class Database;
class Group;
class Instance;

	}

	namespace ui
	{

class PreviewItem;
class PreviewList;
class PreviewSelectionChangeEvent;
class TreeView;
class TreeViewItem;

	}

	namespace editor
	{

class IBrowseFilter;
class IBrowsePreview;
class IEditor;

class BrowseInstanceDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	BrowseInstanceDialog(const IEditor* editor, PropertyGroup* settings);

	bool create(ui::Widget* parent, db::Database* database, const IBrowseFilter* filter);

	virtual void destroy() override final;

	Ref< db::Instance > getInstance();

private:
	const IEditor* m_editor;
	Ref< PropertyGroup > m_settings;
	Ref< ui::TreeView > m_treeDatabase;
	Ref< ui::PreviewList > m_listInstances;
	RefArray< const IBrowsePreview > m_browsePreview;
	Ref< db::Instance > m_instance;

	Thread* m_threadGeneratePreview;
	ThreadsafeFifo< Ref< Functor > > m_previewTasks;
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
}

#endif	// traktor_editor_BrowseInstanceDialog_H
