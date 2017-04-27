/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/TreeViewItem.h"
#include "Ui/Events/ListViewItemActivateEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ListViewItemActivateEvent", ListViewItemActivateEvent, Event)

ListViewItemActivateEvent::ListViewItemActivateEvent(EventSubject* sender, ListViewItem* item)
:	Event(sender)
,	m_item(item)
{
}

ListViewItem* ListViewItemActivateEvent::getItem() const
{
	return m_item;
}

	}
}
