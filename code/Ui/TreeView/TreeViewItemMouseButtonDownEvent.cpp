/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/TreeView/TreeViewItem.h"
#include "Ui/TreeView/TreeViewItemMouseButtonDownEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeViewItemMouseButtonDownEvent", TreeViewItemMouseButtonDownEvent, Event)

TreeViewItemMouseButtonDownEvent::TreeViewItemMouseButtonDownEvent(EventSubject* sender, TreeViewItem* item, int32_t button, const Point& position)
:	Event(sender)
,	m_item(item)
,	m_button(button)
,	m_position(position)
{
}

TreeViewItem* TreeViewItemMouseButtonDownEvent::getItem() const
{
	return m_item;
}

int32_t TreeViewItemMouseButtonDownEvent::getButton() const
{
	return m_button;
}

const Point& TreeViewItemMouseButtonDownEvent::getPosition() const
{
	return m_position;
}

}
