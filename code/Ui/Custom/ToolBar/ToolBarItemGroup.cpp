/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/ToolBar/ToolBarItem.h"
#include "Ui/Custom/ToolBar/ToolBarItemGroup.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ToolBarItemGroup", ToolBarItemGroup, Object)

ToolBarItem* ToolBarItemGroup::addItem(ToolBarItem* item)
{
	m_items.push_back(item);
	return item;
}

void ToolBarItemGroup::removeItem(ToolBarItem* item)
{
	m_items.remove(item);
}

void ToolBarItemGroup::setEnable(bool enable)
{
	for (RefArray< ToolBarItem >::iterator i = m_items.begin(); i != m_items.end(); ++i)
		(*i)->setEnable(enable);
}

		}
	}
}
