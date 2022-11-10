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

/*! Normalization filter.
 * \ingroup Sound
 */
class T_DLLCLASS NormalizationFilter : public IFilter
{
	T_RTTI_CLASS;

public:
	NormalizationFilter(float targetEnergy = 1.0f, float energyThreshold = 0.2f, float attackRate = 0.1f);

	virtual Ref< IFilterInstance > createInstance() const override final;

	virtual void apply(IFilterInstance* instance, SoundBlock& outBlock) const override final;

	virtual void serialize(ISerializer& s) override final;

	inline float getTargetEnergy() const { return m_targetEnergy; }

	inline float getEnergyThreshold() const { return m_energyThreshold; }

	inline float getAttackRate() const { return m_attackRate; }

private:
	float m_targetEnergy;
	float m_energyThreshold;
	float m_attackRate;
};

	}
}

