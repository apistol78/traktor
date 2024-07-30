/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/AudioChannel.h"
#include "Sound/Tracker/SetBpmEvent.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SetBpmEvent", SetBpmEvent, IEvent)

SetBpmEvent::SetBpmEvent(int32_t bpm)
:	m_bpm(bpm)
{
}

bool SetBpmEvent::execute(AudioChannel* audioChannel, int32_t& bpm, int32_t& pattern, int32_t& row) const
{
	bpm = m_bpm;
	return true;
}

}
