/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class Track;

class T_DLLCLASS Pattern : public Object
{
	T_RTTI_CLASS;

public:
	explicit Pattern(int32_t duration, const RefArray< Track >& tracks);

	int32_t getDuration() const { return m_duration; }

	const RefArray< Track >& getTracks() const { return m_tracks; }

private:
	int32_t m_duration;
	RefArray< Track > m_tracks;
};

}
