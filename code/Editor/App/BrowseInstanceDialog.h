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

	namespace ui
	{
		namespace custom
		{

class PreviewItem;
class PreviewList;
class PreviewSelectionChangeEvent;
class TreeView;
class TreeViewItem;

		}
	}

	namespace db
	{

class Database;
class Group;
class Instance;

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

	virtual void destroy() T_OVERRIDE T_FINAL;

	Ref< db::Instance > getInstance();

private:
	const IEditor* m_editor;
	Ref< PropertyGroup > m_settings;
	Ref< ui::custom::TreeView > m_treeDatabase;
	Ref< ui::custom::PreviewList > m_listInstances;
	RefArray< const IBrowsePreview > m_browsePreview;
	Ref< db::Instance > m_instance;

	Thread* m_threadGeneratePreview;
	ThreadsafeFifo< Ref< Functor > > m_previewTasks;
	Event m_previewTaskEvent;

	ui::custom::TreeViewItem* buildGroupItems(ui::custom::TreeView* treeView, ui::custom::TreeViewItem* parent, db::Group* group, const IBrowseFilter* filter);

	void updatePreviewList();

	void eventTreeItemSelected(ui::SelectionChangeEvent* event);

	void eventListItemSelected(ui::custom::PreviewSelectionChangeEvent* event);

	void eventListDoubleClick(ui::MouseDoubleClickEvent* event);

	void taskGeneratePreview(ui::custom::PreviewItem* item);

	void threadGeneratePreview();
};

	}
}

#endif	// traktor_editor_BrowseInstanceDialog_H
