/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Sequencer/SequenceItem.h"
#include "Ui/Sequencer/SequenceMovedEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.SequenceMovedEvent", SequenceMovedEvent, Event)

SequenceMovedEvent::SequenceMovedEvent(EventSubject* sender, SequenceItem* item, int32_t movedTo)
:	Event(sender)
,	m_item(item)
,	m_movedTo(movedTo)
{
}

SequenceItem* SequenceMovedEvent::getItem() const
{
	return m_item;
}

int32_t SequenceMovedEvent::getMovedTo() const
{
	return m_movedTo;
}

}
