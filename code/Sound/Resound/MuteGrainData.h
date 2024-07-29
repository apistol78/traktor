/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Range.h"
#include "Sound/Resound/IGrainData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class T_DLLCLASS MuteGrainData : public IGrainData
{
	T_RTTI_CLASS;

public:
	virtual Ref< IGrain > createInstance(IGrainFactory* grainFactory) const override final;

	virtual void serialize(ISerializer& s) override final;

	const Range< float >& getDuration() const { return m_duration; }

private:
	Range< float > m_duration = { 0.0f, 0.0f };
};

}
