/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Sequencer/KeyMoveEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.KeyMoveEvent", KeyMoveEvent, Event)

KeyMoveEvent::KeyMoveEvent(EventSubject* sender, Key* key, int32_t offset)
:	Event(sender)
,	m_key(key)
,	m_offset(offset)
{
}

Key* KeyMoveEvent::getKey() const
{
	return m_key;
}

int32_t KeyMoveEvent::getOffset() const
{
	return m_offset;
}

}
