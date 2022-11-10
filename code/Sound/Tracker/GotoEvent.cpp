/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/AudioChannel.h"
#include "Sound/Tracker/GotoEvent.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.GotoEvent", GotoEvent, IEvent)

GotoEvent::GotoEvent(int32_t pattern, int32_t row)
:	m_pattern(pattern)
,	m_row(row)
{
}

bool GotoEvent::execute(AudioChannel* audioChannel, int32_t& bpm, int32_t& pattern, int32_t& row) const
{
	pattern = m_pattern;
	row = m_row;
	return true;
}

	}
}
