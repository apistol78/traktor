/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Events/IdleEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.IdleEvent", IdleEvent, Event)

IdleEvent::IdleEvent(EventSubject* sender)
:	Event(sender)
,	m_requestMore(false)
{
}

void IdleEvent::requestMore()
{
	m_requestMore = true;
}

bool IdleEvent::requestedMore() const
{
	return m_requestMore;
}

	}
}
