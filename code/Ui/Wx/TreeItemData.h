#ifndef traktor_ui_TreeItemData_H
#define traktor_ui_TreeItemData_H

#include <wx/treectrl.h>

namespace traktor
{
	namespace ui
	{

class TreeItemData : public wxTreeItemData
{
public:
	TreeItemData(TreeViewItemWx* item)
	:	m_item(item)
	{
	}

	TreeViewItemWx* getItem()
	{
		return m_item;
	}

private:
	Ref< TreeViewItemWx > m_item;
};

	}
}

#endif	// traktor_ui_TreeItemData_H
