/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/IAudioFilter.h"
#include "Core/Math/Random.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

/*! Dither filter.
 * \ingroup Sound
 */
class T_DLLCLASS DitherFilter : public IAudioFilter
{
	T_RTTI_CLASS;

public:
	explicit DitherFilter(uint32_t bitsPerSample = 16);

	virtual Ref< IAudioFilterInstance > createInstance() const override final;

	virtual void apply(IAudioFilterInstance* instance, AudioBlock& outBlock) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	float m_ditherAmplitude;
	mutable Random m_random;
};

}
