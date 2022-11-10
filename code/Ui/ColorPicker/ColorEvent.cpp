/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/ColorPicker/ColorEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ColorEvent", ColorEvent, Event)

ColorEvent::ColorEvent(EventSubject* sender, const Color4ub& color)
:	Event(sender)
,	m_color(color)
{
}

const Color4ub& ColorEvent::getColor() const
{
	return m_color;
}

	}
}
