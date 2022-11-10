/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Editor/Ui/TargetCaptureEvent.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetCaptureEvent", TargetCaptureEvent, ui::Event)

TargetCaptureEvent::TargetCaptureEvent(ui::EventSubject* sender, TargetInstance* instance, int32_t connectionIndex)
:	Event(sender)
,	m_instance(instance)
,	m_connectionIndex(connectionIndex)
{
}

TargetInstance* TargetCaptureEvent::getInstance() const
{
	return m_instance;
}

int32_t TargetCaptureEvent::getConnectionIndex() const
{
	return m_connectionIndex;
}

	}
}
