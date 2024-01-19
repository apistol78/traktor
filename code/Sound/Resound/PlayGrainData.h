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
#include "Core/Math/Range.h"
#include "Resource/Id.h"
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

class IAudioFilter;
class Sound;

class T_DLLCLASS PlayGrainData : public IGrainData
{
	T_RTTI_CLASS;

public:
	PlayGrainData();

	virtual Ref< IGrain > createInstance(IGrainFactory* grainFactory) const override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< Sound >& getSound() const { return m_sound; }

	const Range< float >& getGain() const { return m_gain; }

	const Range< float >& getPitch() const { return m_pitch; }

	bool getRepeat() const { return m_repeat; }

private:
	resource::Id< Sound > m_sound;
	RefArray< IAudioFilter > m_filters;
	Range< float > m_gain;
	Range< float > m_pitch;
	bool m_repeat;
};

	}
}

