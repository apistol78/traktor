/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeHdr.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "World/Entity/FogComponent.h"
#include "World/Entity/FogComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.FogComponentData", 2, FogComponentData, IWorldComponentData)

Ref< FogComponent > FogComponentData::createComponent() const
{
	return new FogComponent(this);
}

void FogComponentData::serialize(ISerializer& s)
{
	if (s.getVersion< FogComponentData >() >= 2)
	{
		s >> Member< Color4f >(L"mediumColor", m_mediumColor);
		s >> Member< float >(L"mediumDensity", m_mediumDensity, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
		s >> Member< bool >(L"distanceFogEnable", m_distanceFogEnable);
	}

	s >> Member< float >(L"fogDistance", m_fogDistance, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"fogElevation", m_fogElevation, AttributeUnit(UnitType::Metres));

	if (s.getVersion< FogComponentData >() < 2)
	{
		s >> ObsoleteMember< float >(L"fogDensity");
		s >> ObsoleteMember< float >(L"fogDensityMax");
		s >> ObsoleteMember< Color4f >(L"fogColor");
	}

	s >> Member< bool >(L"volumetricFogEnable", m_volumetricFogEnable);
	s >> Member< float >(L"maxDistance", m_maxDistance, AttributeRange(0.0f));
	s >> Member< float >(L"maxScattering", m_maxScattering, AttributeRange(0.0f));
	
	if (s.getVersion< FogComponentData >() < 1)
		s >> ObsoleteMember< int32_t >(L"sliceCount");

	if (s.getVersion< FogComponentData >() < 2)
	{
		s >> Member< Color4f >(L"mediumColor", m_mediumColor);
		s >> Member< float >(L"mediumDensity", m_mediumDensity, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
	}
}

}
