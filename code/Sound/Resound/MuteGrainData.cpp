/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Random.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Sound/Resound/MuteGrain.h"
#include "Sound/Resound/MuteGrainData.h"

namespace traktor::sound
{
	namespace
	{

Random s_random;

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.MuteGrainData", 1, MuteGrainData, IGrainData)

Ref< IGrain > MuteGrainData::createInstance(IGrainFactory* grainFactory) const
{
	return new MuteGrain(m_duration.random(s_random));
}

void MuteGrainData::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		s >> MemberComposite< Range< float > >(L"duration", m_duration);
	else
	{
		double duration;
		s >> Member< double >(L"duration", duration);
		m_duration = Range< float >(float(duration), float(duration));
	}
}

}
