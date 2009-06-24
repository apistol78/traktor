#ifndef traktor_editor_BrowseInstanceDialog_H
#define traktor_editor_BrowseInstanceDialog_H

#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace ui
	{

class TreeView;
class TreeViewItem;
class ListView;

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

class BrowseInstanceDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS(BrowseInstanceDialog)

public:
	bool create(ui::Widget* parent, db::Database* database, const IBrowseFilter* filter);

	db::Instance* getInstance();

private:
	Ref< ui::TreeView > m_treeDatabase;
	Ref< ui::ListView > m_listInstances;
	Ref< db::Instance > m_instance;

	void buildGroupItems(ui::TreeView* treeView, ui::TreeViewItem* parent, db::Group* group, const IBrowseFilter* filter);

	void eventTreeItemSelected(ui::Event* event);

	void eventListItemSelected(ui::Event* event);

	void eventListDoubleClick(ui::Event* event);
};

	}
}

#endif	// traktor_editor_BrowseInstanceDialog_H
