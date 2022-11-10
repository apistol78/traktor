/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/Tracker/Play.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.Play", Play, Object)

Play::Play(const resource::Proxy< Sound >& sound, int32_t note, int32_t repeatFrom, int32_t repeatLength)
:	m_sound(sound)
,	m_note(note)
,	m_repeatFrom(repeatFrom)
,	m_repeatLength(repeatLength)
{
}

	}
}
