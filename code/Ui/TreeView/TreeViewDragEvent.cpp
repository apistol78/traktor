/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/TreeView/TreeViewDragEvent.h"
#include "Ui/TreeView/TreeViewItem.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeViewDragEvent", TreeViewDragEvent, DragEvent)

TreeViewDragEvent::TreeViewDragEvent(EventSubject* sender, TreeViewItem* dragItem, Moment moment, const Point& position)
:	DragEvent(sender, moment, position)
,	m_dragItem(dragItem)
{
}

TreeViewItem* TreeViewDragEvent::getItem() const
{
	return m_dragItem;
}

}
