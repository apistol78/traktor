/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/TreeView/TreeViewItem.h"
#include "Ui/TreeView/TreeViewEditEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeViewEditEvent", TreeViewEditEvent, Event)

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
