/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Events/MouseWheelEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MouseWheelEvent", MouseWheelEvent, Event)

MouseWheelEvent::MouseWheelEvent(EventSubject* sender, int32_t rotation, const ui::Point& position)
:	Event(sender)
,	m_rotation(rotation)
,	m_position(position)
{
}

int32_t MouseWheelEvent::getRotation() const
{
	return m_rotation;
}

const ui::Point& MouseWheelEvent::getPosition() const
{
	return m_position;
}

	}
}
