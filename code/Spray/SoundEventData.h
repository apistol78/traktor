/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Resource/Id.h"
#include "World/IEntityEventData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class Sound;

	}

	namespace spray
	{

/*! Sound event persistent data.
 * \ingroup Spray
 */
class T_DLLCLASS SoundEventData : public world::IEntityEventData
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

private:
	friend class EffectEntityFactory;
	friend class EffectEntityPipeline;

	resource::Id< sound::Sound > m_sound;
	bool m_positional = true;
	bool m_follow = true;
	bool m_autoStopFar = true;
};

	}
}

