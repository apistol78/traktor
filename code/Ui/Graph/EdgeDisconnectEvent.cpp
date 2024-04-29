/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Graph/EdgeDisconnectEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.EdgeDisconnectEvent", EdgeDisconnectEvent, Event)

EdgeDisconnectEvent::EdgeDisconnectEvent(EventSubject* sender, Edge* edge)
:	Event(sender)
,	m_edge(edge)
{
}

Edge* EdgeDisconnectEvent::getEdge() const
{
	return m_edge;
}

}
