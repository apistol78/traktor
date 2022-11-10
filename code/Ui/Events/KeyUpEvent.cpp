/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Events/KeyUpEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.KeyUpEvent", KeyUpEvent, Event)

KeyUpEvent::KeyUpEvent(
	EventSubject* sender,
	VirtualKey virtualKey,
	uint32_t systemKey,
	wchar_t character,
	bool repeat
)
:	Event(sender)
,	m_virtualKey(virtualKey)
,	m_systemKey(systemKey)
,	m_character(character)
,	m_repeat(repeat)
{
}

VirtualKey KeyUpEvent::getVirtualKey() const
{
	return m_virtualKey;
}

uint32_t KeyUpEvent::getSystemKey() const
{
	return m_systemKey;
}

wchar_t KeyUpEvent::getCharacter() const
{
	return m_character;
}

bool KeyUpEvent::isRepeat() const
{
	return m_repeat;
}

	}
}
