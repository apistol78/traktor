/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Events/SelectionChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.SelectionChangeEvent", SelectionChangeEvent, Event)

SelectionChangeEvent::SelectionChangeEvent(EventSubject* sender)
:	Event(sender)
{
}

SelectionChangeEvent::SelectionChangeEvent(EventSubject* sender, Object* item)
:	Event(sender)
,	m_item(item)
{
}

Object* SelectionChangeEvent::getItem() const
{
	return m_item;
}

	}
}
