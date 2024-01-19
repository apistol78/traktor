/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/IAudioFilter.h"

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

/*! Comb filter.
 * \ingroup Sound
 */
class T_DLLCLASS CombFilter : public IAudioFilter
{
	T_RTTI_CLASS;

public:
	CombFilter(uint32_t samplesLength = 10, float feedback = 0.0f, float damp = 0.0f);

	virtual Ref< IAudioFilterInstance > createInstance() const override final;

	virtual void apply(IAudioFilterInstance* instance, AudioBlock& outBlock) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_samplesLength;
	float m_feedback;
	float m_damp;
};

	}
}

