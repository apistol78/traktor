/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/TreeView/TreeViewDragEvent.h"
#include "Ui/Custom/TreeView/TreeViewItem.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.TreeViewDragEvent", TreeViewDragEvent, Event)

TreeViewDragEvent::TreeViewDragEvent(EventSubject* sender, TreeViewItem* dragItem, DragMoment moment, const Point& position)
:	Event(sender)
,	m_dragItem(dragItem)
,	m_moment(moment)
,	m_position(position)
,	m_cancelled(false)
{
}

TreeViewItem* TreeViewDragEvent::getItem() const
{
	return m_dragItem;
}

TreeViewDragEvent::DragMoment TreeViewDragEvent::getMoment() const
{
	return m_moment;
}

const Point& TreeViewDragEvent::getPosition() const
{
	return m_position;
}

void TreeViewDragEvent::cancel()
{
	m_cancelled = true;
}

bool TreeViewDragEvent::cancelled() const
{
	return m_cancelled;
}

		}
	}
}
