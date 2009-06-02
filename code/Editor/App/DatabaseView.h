#ifndef traktor_editor_DatabaseView_H
#define traktor_editor_DatabaseView_H

#include "Core/Heap/Ref.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{

class TreeView;
class TreeViewItem;
class PopupMenu;

		namespace custom
		{

class ToolBar;
class ToolBarButton;

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

class Editor;
class WizardTool;

class DatabaseView : public ui::Container
{
	T_RTTI_CLASS(EditorPane)

public:
	class Filter : public Object
	{
		T_RTTI_CLASS(Filter)

	public:
		virtual bool acceptInstance(const db::Instance* instance) const = 0;

		virtual bool acceptEmptyGroups() const = 0;
	};

	DatabaseView(Editor* editor);

	bool create(ui::Widget* parent);

	void destroy();

	void setDatabase(db::Database* db);

	void updateView();

private:
	Editor* m_editor;
	Ref< ui::custom::ToolBar > m_toolSelection;
	Ref< ui::custom::ToolBarButton > m_toolFilter;
	Ref< ui::TreeView > m_treeDatabase;
	Ref< ui::PopupMenu > m_menuGroup;
	Ref< ui::PopupMenu > m_menuInstance;
	Ref< ui::PopupMenu > m_menuInstanceAsset;
	Ref< db::Database > m_db;
	Ref< Filter > m_filter;
	RefArray< WizardTool > m_wizardTools;

	ui::TreeViewItem* buildTreeItem(ui::TreeView* treeView, ui::TreeViewItem* parentItem, db::Group* group);

	void filterType(db::Instance* instance);

	void filterDependencies(db::Instance* instance);

	void eventToolSelectionClicked(ui::Event* event);

	void eventInstanceActivate(ui::Event* event);

	void eventInstanceButtonDown(ui::Event* event);

	void eventInstanceRenamed(ui::Event* event);

	void eventInstanceDrag(ui::Event* event);
};

	}
}

#endif	// traktor_editor_DatabaseView_H
