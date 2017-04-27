/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/TreeViewItem.h"
#include "Ui/Events/TreeViewItemActivateEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeViewItemActivateEvent", TreeViewItemActivateEvent, Event)

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
