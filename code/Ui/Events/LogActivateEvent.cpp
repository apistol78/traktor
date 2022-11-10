/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Events/LogActivateEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.LogActivateEvent", LogActivateEvent, Event)

LogActivateEvent::LogActivateEvent(EventSubject* sender, const Guid& symbolId)
:	Event(sender)
,	m_symbolId(symbolId)
{
}

const Guid& LogActivateEvent::getSymbolId() const
{
	return m_symbolId;
}

	}
}
