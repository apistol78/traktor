/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Events/PaintEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PaintEvent", PaintEvent, Event)

PaintEvent::PaintEvent(EventSubject* sender, Canvas& canvas, const Rect& rc)
:	Event(sender)
,	m_canvas(canvas)
,	m_rc(rc)
{
}

Canvas& PaintEvent::getCanvas() const
{
	return m_canvas;
}

const Rect& PaintEvent::getUpdateRect() const
{
	return m_rc;
}

}
