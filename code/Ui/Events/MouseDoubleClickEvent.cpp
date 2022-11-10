/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Events/MouseDoubleClickEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MouseDoubleClickEvent", MouseDoubleClickEvent, Event)

MouseDoubleClickEvent::MouseDoubleClickEvent(EventSubject* sender, int32_t button, const ui::Point& position)
:	Event(sender)
,	m_button(button)
,	m_position(position)
{
}

int32_t MouseDoubleClickEvent::getButton() const
{
	return m_button;
}

const ui::Point& MouseDoubleClickEvent::getPosition() const
{
	return m_position;
}

	}
}
