/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
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

namespace traktor::sound
{

/*! IIR filter.
 * \ingroup Sound
 */
class T_DLLCLASS IIRFilter : public IAudioFilter
{
	T_RTTI_CLASS;

public:
	enum class FilterType
	{
		Butterworth,
		Chebyshev,
		Elliptic
	};

	enum class FilterMode
	{
		LowPass,
		HighPass
	};

	virtual Ref< IAudioFilterInstance > createInstance() const override final;

	virtual void apply(IAudioFilterInstance* instance, AudioBlock& outBlock) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	FilterType m_filterType = FilterType::Butterworth;
	FilterMode m_filterMode = FilterMode::LowPass;
	float m_cutOffFrequency = 0.0f;
	float m_stopFrequency = 0.0f;
	float m_passBand = 1.0f;
	float m_stopBand = -50.0f;
};

}
