/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Sound/Resound/RepeatGrain.h"
#include "Sound/Resound/RepeatGrainData.h"
#include "Sound/Resound/IGrainFactory.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.RepeatGrainData", 0, RepeatGrainData, IGrainData)

Ref< IGrain > RepeatGrainData::createInstance(IGrainFactory* grainFactory) const
{
	Ref< IGrain > grain = grainFactory->createInstance(m_grain);
	if (!grain)
		return nullptr;

	return new RepeatGrain(m_count, grain);
}

void RepeatGrainData::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"count", m_count);
	s >> MemberRef< IGrainData >(L"grain", m_grain);
}

}
