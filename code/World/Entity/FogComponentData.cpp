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
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "World/Entity/FogComponent.h"
#include "World/Entity/FogComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.FogComponentData", 5, FogComponentData, IWorldComponentData)

const Guid FogComponentData::ms_generatedShaderSeed(L"{AB509CD4-793A-4EF4-AEDD-E08909B1FC00}");

Ref< FogComponent > FogComponentData::createComponent(resource::IResourceManager* resourceManager) const
{
	const Guid outputGuid = m_mediumShader.permutation(ms_generatedShaderSeed);

	resource::Proxy< render::Shader > mediumShader;
	if (!resourceManager->bind(resource::Id< render::Shader >(outputGuid), mediumShader))
		return nullptr;

	return new FogComponent(this, mediumShader);
}

void FogComponentData::serialize(ISerializer& s)
{
	if (s.getVersion< FogComponentData >() >= 2)
	{
		s >> Member< Color4f >(L"mediumColor", m_mediumColor);
		s >> Member< float >(L"mediumDensity", m_mediumDensity, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));

		if (s.getVersion< FogComponentData >() >= 5)
			s >> Member< Guid >(L"mediumShader", m_mediumShader);

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

	if (s.getVersion< FogComponentData >() < 4)
		s >> ObsoleteMember< float >(L"maxScattering");

	if (s.getVersion< FogComponentData >() >= 3)
	{
		s >> Member< float >(L"phaseForward", m_phaseForward, AttributeRange(-0.95f, 0.95f));
		s >> Member< float >(L"phaseBackward", m_phaseBackward, AttributeRange(-0.95f, 0.95f));
		s >> Member< float >(L"phaseBlend", m_phaseBlend, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
	}
	
	if (s.getVersion< FogComponentData >() < 1)
		s >> ObsoleteMember< int32_t >(L"sliceCount");

	if (s.getVersion< FogComponentData >() < 2)
	{
		s >> Member< Color4f >(L"mediumColor", m_mediumColor);
		s >> Member< float >(L"mediumDensity", m_mediumDensity, AttributeRange(0.0f, 1.0f) | AttributeUnit(UnitType::Percent));
	}
}

}
