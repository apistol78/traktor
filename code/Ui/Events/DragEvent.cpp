/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Events/DragEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.DragEvent", DragEvent, Event)

DragEvent::DragEvent(EventSubject* sender, Moment moment, const Point& position)
:	Event(sender)
,	m_moment(moment)
,	m_position(position)
,	m_cancelled(false)
{
}

DragEvent::Moment DragEvent::getMoment() const
{
	return m_moment;
}

const Point& DragEvent::getPosition() const
{
	return m_position;
}

void DragEvent::cancel()
{
	m_cancelled = true;
}

bool DragEvent::cancelled() const
{
	return m_cancelled;
}

}
