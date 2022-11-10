/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class Sound;

class T_DLLCLASS Play : public Object
{
	T_RTTI_CLASS;

public:
	Play(const resource::Proxy< Sound >& sound, int32_t note, int32_t repeatFrom, int32_t repeatLength);

	const resource::Proxy< Sound >& getSound() const { return m_sound; }

	int32_t getNote() const { return m_note; }

	int32_t getRepeatFrom() const { return m_repeatFrom; }

	int32_t getRepeatLength() const { return m_repeatLength; }

private:
	resource::Proxy< Sound > m_sound;
	int32_t m_note;
	int32_t m_repeatFrom;
	int32_t m_repeatLength;
};

	}
}
