/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/TreeView/TreeViewDragEvent.h"
#include "Ui/TreeView/TreeViewItem.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeViewDragEvent", TreeViewDragEvent, Event)

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
