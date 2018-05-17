/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_DatabaseView_H
#define traktor_editor_DatabaseView_H

#include "Core/RefArray.h"
#include "Ui/Container.h"

namespace traktor
{

class PropertyGroup;

	namespace ui
	{

class Edit;
class HierarchicalState;
class PopupMenu;

		namespace custom
		{

class GridRowDoubleClickEvent;
class GridView;
class Splitter;
class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;
class ToolBarDropDown;
class TreeView;
class TreeViewContentChangeEvent;
class TreeViewDragEvent;
class TreeViewItem;
class TreeViewItemActivateEvent;
class TreeViewItemStateChangeEvent;

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

class IEditor;
class IWizardTool;

class DatabaseView : public ui::Container
{
	T_RTTI_CLASS;

public:
	class Filter : public Object
	{
		T_RTTI_CLASS;

	public:
		virtual bool acceptInstance(const db::Instance* instance) const = 0;

		virtual bool acceptEmptyGroups() const = 0;
	};

	DatabaseView(IEditor* editor);

	bool create(ui::Widget* parent);

	virtual void destroy() T_OVERRIDE T_FINAL;

	void setDatabase(db::Database* db);

	void updateView();

	bool highlight(const db::Instance* instance);

	bool handleCommand(const ui::Command& command);

	virtual void setEnable(bool enable) T_OVERRIDE T_FINAL;

private:
	IEditor* m_editor;
	Ref< ui::custom::ToolBar > m_toolSelection;
	Ref< ui::custom::ToolBarButton > m_toolFilterType;
	Ref< ui::custom::ToolBarButton > m_toolFilterAssets;
	Ref< ui::custom::ToolBarButton > m_toolFilterShow;
	Ref< ui::custom::ToolBarButton > m_toolFavoritesShow;
	Ref< ui::custom::ToolBarDropDown > m_toolViewMode;
	Ref< ui::Edit > m_editFilter;
	Ref< ui::custom::Splitter > m_splitter;
	Ref< ui::custom::TreeView > m_treeDatabase;
	Ref< ui::custom::GridView > m_gridInstances;
	Ref< ui::HierarchicalState > m_treeState;
	Ref< ui::PopupMenu > m_menuGroup[2];
	Ref< ui::PopupMenu > m_menuInstance;
	Ref< ui::PopupMenu > m_menuInstanceAsset;
	Ref< PropertyGroup > m_iconsGroup;
	Ref< db::Database > m_db;
	Ref< Filter > m_filter;
	RefArray< IWizardTool > m_wizardTools;
	std::set< Guid > m_rootInstances;
	std::set< Guid > m_favoriteInstances;
	std::wstring m_filterText;
	int32_t m_filterCountDown;
	int32_t m_colorCountDown;

	int32_t getIconIndex(const TypeInfo* instanceType) const;

	Ref< ui::custom::TreeViewItem > buildTreeItem(ui::custom::TreeView* treeView, ui::custom::TreeViewItem* parentItem, db::Group* group);

	Ref< ui::custom::TreeViewItem > buildTreeItemSplit(ui::custom::TreeView* treeView, ui::custom::TreeViewItem* parentItem, db::Group* group);

	void updateTreeColors();

	void updateGridInstances();

	void filterType(db::Instance* instance);

	void filterDependencies(db::Instance* instance);

	void eventToolSelectionClicked(ui::custom::ToolBarButtonClickEvent* event);

	void eventFilterKey(ui::KeyUpEvent* event);

	void eventTimer(ui::TimerEvent* event);

	void eventInstanceActivate(ui::custom::TreeViewItemActivateEvent* event);

	void eventInstanceStateChange(ui::custom::TreeViewItemStateChangeEvent* event);

	void eventInstanceSelect(ui::SelectionChangeEvent* event);

	void eventInstanceButtonDown(ui::MouseButtonDownEvent* event);

	void eventInstanceRenamed(ui::custom::TreeViewContentChangeEvent* event);

	void eventInstanceDrag(ui::custom::TreeViewDragEvent* event);

	void eventInstanceGridActivate(ui::custom::GridRowDoubleClickEvent* event);
};

	}
}

#endif	// traktor_editor_DatabaseView_H
