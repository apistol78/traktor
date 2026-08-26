/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Sequencer/KeySelectEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.KeySelectEvent", KeySelectEvent, Event)

KeySelectEvent::KeySelectEvent(EventSubject* sender, Sequence* sequence, Key* key)
:	Event(sender)
,	m_sequence(sequence)
,	m_key(key)
{
}

Sequence* KeySelectEvent::getSequence() const
{
	return m_sequence;
}

Key* KeySelectEvent::getKey() const
{
	return m_key;
}

}
