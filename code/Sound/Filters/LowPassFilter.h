/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/IFilter.h"

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

/*! Low pass filter.
 * \ingroup Sound
 */
class T_DLLCLASS LowPassFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	LowPassFilter(float cutOff = 1e+8f);

	virtual Ref< IFilterInstance > createInstance() const override final;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const override final;

	virtual void serialize(ISerializer& s) override final;

	inline void setCutOff(float cutOff) { m_cutOff = cutOff; }

	inline float getCutOff() const { return m_cutOff; }

private:
	float m_cutOff;
};

	}
}

