/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
