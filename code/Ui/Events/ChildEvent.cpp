/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Widget.h"
#include "Ui/Events/ChildEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ChildEvent", ChildEvent, Event)

ChildEvent::ChildEvent(EventSubject* sender, Widget* parent, Widget* child, bool link)
:	Event(sender)
,	m_parent(parent)
,	m_child(child)
,	m_link(link)
{
}

Widget* ChildEvent::getParent() const
{
	return m_parent;
}

Widget* ChildEvent::getChild() const
{
	return m_child;
}

bool ChildEvent::link() const
{
	return m_link;
}

	}
}
