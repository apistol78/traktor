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
#include "Core/Ref.h"
#include "Sound/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class ISoundBuffer;

/*! Sound container class.
 * \ingroup Sound
 */
class T_DLLCLASS Sound : public Object
{
	T_RTTI_CLASS;

public:
	explicit Sound(
		ISoundBuffer* buffer,
		handle_t category,
		float gain,
		float range
	);

	ISoundBuffer* getBuffer() const { return m_buffer; }

	uint32_t getCategory() const { return m_category; }

	float getGain() const { return m_gain; }

	float getRange() const { return m_range; }

private:
	Ref< ISoundBuffer > m_buffer;
	handle_t m_category;
	float m_gain;
	float m_range;
};

}
