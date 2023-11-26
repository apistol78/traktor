/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/Resound/IGrainFactory.h"
#include "Sound/Resound/SimultaneousGrain.h"
#include "Sound/Resound/SimultaneousGrainData.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SimultaneousGrainData", 0, SimultaneousGrainData, IGrainData)

Ref< IGrain > SimultaneousGrainData::createInstance(IGrainFactory* grainFactory) const
{
	RefArray< IGrain > grains;

	grains.resize(m_grains.size());
	for (uint32_t i = 0; i < m_grains.size(); ++i)
	{
		grains[i] = grainFactory->createInstance(m_grains[i]);
		if (!grains[i])
			return nullptr;
	}

	return new SimultaneousGrain(grains);
}

void SimultaneousGrainData::serialize(ISerializer& s)
{
	s >> MemberRefArray< IGrainData >(L"grains", m_grains, AttributePrivate());
}

}
