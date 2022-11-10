/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Events/SizeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.SizeEvent", SizeEvent, Event)

SizeEvent::SizeEvent(EventSubject* sender, const Size& size)
:	Event(sender)
,	m_size(size)
{
}

const Size& SizeEvent::getSize() const
{
	return m_size;
}

int32_t SizeEvent::getWidth() const
{
	return m_size.cx;
}

int32_t SizeEvent::getHeight() const
{
	return m_size.cy;
}

	}
}
