/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Sound/Resound/IGrainData.h"

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

class T_DLLCLASS SequenceGrainData : public IGrainData
{
	T_RTTI_CLASS;

public:
	virtual Ref< IGrain > createInstance(IGrainFactory* grainFactory) const override final;

	virtual void serialize(ISerializer& s) override final;

	void addGrain(IGrainData* grain) { m_grains.push_back(grain); }

	void removeGrain(IGrainData* grain) { m_grains.remove(grain); }

	const RefArray< IGrainData >& getGrains() const { return m_grains; }

private:
	RefArray< IGrainData > m_grains;
};

	}
}

