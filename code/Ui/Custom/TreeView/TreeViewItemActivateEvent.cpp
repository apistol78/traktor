/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/TreeView/TreeViewItem.h"
#include "Ui/Custom/TreeView/TreeViewItemActivateEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.TreeViewItemActivateEvent", TreeViewItemActivateEvent, Event)

TreeViewItemActivateEvent::TreeViewItemActivateEvent(EventSubject* sender, TreeViewItem* item)
:	Event(sender)
,	m_item(item)
{
}

TreeViewItem* TreeViewItemActivateEvent::getItem() const
{
	return m_item;
}

		}
	}
}
