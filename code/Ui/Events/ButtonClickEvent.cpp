/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Events/ButtonClickEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ButtonClickEvent", ButtonClickEvent, Event)

ButtonClickEvent::ButtonClickEvent(EventSubject* sender, const Command& command)
:	Event(sender)
,	m_command(command)
{
}

ButtonClickEvent::ButtonClickEvent(EventSubject* sender)
:	Event(sender)
{
}

const Command& ButtonClickEvent::getCommand() const
{
	return m_command;
}

	}
}
