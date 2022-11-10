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

/*! Audio mixer interface.
 * \ingroup Sound
 *
 * Following restrictions for use with this interface apply:
 * - Sample blocks must be 16-byte aligned.
 * - Counts must be a multiple of 4.
 */
class T_DLLCLASS IAudioMixer : public Object
{
	T_RTTI_CLASS;

public:
	virtual void mulConst(float* sb, uint32_t count, float factor) const = 0;

	virtual void mulConst(float* lsb, const float* rsb, uint32_t count, float factor) const = 0;

	virtual void addMulConst(float* lsb, const float* rsb, uint32_t count, float factor) const = 0;

	virtual void stretch(float* lsb, uint32_t lcount, const float* rsb, uint32_t rcount, float factor) const = 0;

	virtual void mute(float* sb, uint32_t count) const = 0;

	virtual void synchronize() const = 0;
};

	}
}

