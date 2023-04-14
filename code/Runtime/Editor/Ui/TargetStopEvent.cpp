/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Editor/Ui/TargetStopEvent.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetStopEvent", TargetStopEvent, ui::Event)

TargetStopEvent::TargetStopEvent(ui::EventSubject* sender, TargetInstance* instance, int32_t connectionIndex)
:	Event(sender)
,	m_instance(instance)
,	m_connectionIndex(connectionIndex)
{
}

TargetInstance* TargetStopEvent::getInstance() const
{
	return m_instance;
}

int32_t TargetStopEvent::getConnectionIndex() const
{
	return m_connectionIndex;
}

}
