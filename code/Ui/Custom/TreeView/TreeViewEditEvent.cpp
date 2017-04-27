/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/TreeView/TreeViewItem.h"
#include "Ui/Custom/TreeView/TreeViewEditEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.TreeViewEditEvent", TreeViewEditEvent, Event)

TreeViewEditEvent::TreeViewEditEvent(EventSubject* sender, TreeViewItem* item)
:	Event(sender)
,	m_item(item)
,	m_cancelled(false)
{
}

TreeViewItem* TreeViewEditEvent::getItem() const
{
	return m_item;
}

void TreeViewEditEvent::cancel()
{
	m_cancelled = true;
}

bool TreeViewEditEvent::cancelled() const
{
	return m_cancelled;
}

		}
	}
}
