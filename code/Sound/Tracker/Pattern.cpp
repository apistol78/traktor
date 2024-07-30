/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/Tracker/Pattern.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.Pattern", Pattern, Object)

Pattern::Pattern(int32_t duration, const RefArray< Track >& tracks)
:	m_duration(duration)
,	m_tracks(tracks)
{
}

}
