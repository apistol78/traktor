/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/Murmur3.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Shape/Editor/Bake/BakeOperationData.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.BakeOperationData", 0, BakeOperationData, scene::ISceneOperationData)

uint32_t BakeOperationData::calculateModelRelevanteHash() const
{
	Murmur3 cs;
	cs.begin();
	cs.feed(m_enableLightmaps);
	cs.feed(m_lumelDensity);
	cs.feed(m_minimumLightMapSize);
	cs.feed(m_maximumLightMapSize);
	cs.end();
	return cs.get();
}

void BakeOperationData::serialize(ISerializer& s)
{
	s >> Member< bool >(L"enableLightmaps", m_enableLightmaps);
	s >> Member< uint32_t >(L"primarySampleCount", m_primarySampleCount, AttributeRange(0));
	s >> Member< uint32_t >(L"secondarySampleCount", m_secondarySampleCount, AttributeRange(0));
	s >> Member< uint32_t >(L"shadowSampleCount", m_shadowSampleCount, AttributeRange(0));
	s >> Member< float >(L"maxPathDistance", m_maxPathDistance, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"pointLightShadowRadius", m_pointLightShadowRadius, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"lumelDensity", m_lumelDensity, AttributeRange(0.0f));
	s >> Member< Vector4 >(L"irradianceGridDensity", m_irradianceGridDensity, AttributeDirection() | AttributeRange(0.0f));
	s >> Member< int32_t >(L"minimumLightMapSize", m_minimumLightMapSize, AttributeRange(0));
	s >> Member< int32_t >(L"maximumLightMapSize", m_maximumLightMapSize, AttributeRange(0));
	s >> Member< bool >(L"enableDenoise", m_enableDenoise);
	s >> Member< float >(L"analyticalLightAttenuation", m_analyticalLightAttenuation, AttributeRange(0.0f) | AttributeUnit(UnitType::Percent));
	s >> Member< float >(L"ambientOcclusionFactor", m_ambientOcclusionFactor, AttributeRange(0.0f) | AttributeUnit(UnitType::Percent));
}

}
