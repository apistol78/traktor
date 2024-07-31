/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/ToolTipEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolTipEvent", ToolTipEvent, Event)

ToolTipEvent::ToolTipEvent(EventSubject* sender, const Point& position)
:	Event(sender)
,	m_position(position)
{
}

void ToolTipEvent::setPosition(const Point& position)
{
	m_position = position;
}

const Point& ToolTipEvent::getPosition() const
{
	return m_position;
}

void ToolTipEvent::setText(const std::wstring& text)
{
	m_text = text;
}

const std::wstring& ToolTipEvent::getText() const
{
	return m_text;
}

}
