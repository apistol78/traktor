/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Events/ReconfigureEvent.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.ReconfigureEvent", ReconfigureEvent, Object)

ReconfigureEvent::ReconfigureEvent(bool finished, int32_t result)
:	m_finished(finished)
,	m_result(result)
{
}

bool ReconfigureEvent::isFinished() const
{
	return m_finished;
}

int32_t ReconfigureEvent::getResult() const
{
	return m_result;
}

}
