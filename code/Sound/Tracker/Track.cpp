/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/Tracker/Track.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.Track", Track, Object)

const Track::Key* Track::findKey(int32_t position) const
{
	for (const auto& key : m_keys)
	{
		if (key.at == position)
			return &key;
	}
	return nullptr;
}

	}
}
