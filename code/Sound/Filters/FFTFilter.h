/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Sound/IAudioFilter.h"
#include "Core/Math/Random.h"

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

/*! FFT filter.
 * \ingroup Sound
 */
class T_DLLCLASS FFTFilter : public IAudioFilter
{
	T_RTTI_CLASS;

public:
	enum { N = 4096 };

	FFTFilter(uint32_t sampleRate = 44100);

	/*!
	 * Set per frequency filter gain.
	 * \param filter Pairs of frequency (Hz) and gain.
	 * \param windowWidth Hanning window width (0 means no windowing).
	 */
	void setFilter(const std::vector< std::pair< float, float > >& filter, uint32_t windowWidth);

	virtual Ref< IAudioFilterInstance > createInstance() const override final;

	virtual void apply(IAudioFilterInstance* instance, AudioBlock& outBlock) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_sampleRate;
	float m_filter[N];
};

	}
}

