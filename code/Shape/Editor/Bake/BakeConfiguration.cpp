/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/Murmur3.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.BakeConfiguration", 32, BakeConfiguration, ISerializable)

uint32_t BakeConfiguration::calculateModelRelevanteHash() const
{
	Murmur3 cs;
	cs.begin();
	cs.feed(m_lumelDensity);
	cs.feed(m_minimumLightMapSize);
	cs.feed(m_maximumLightMapSize);
	cs.end();
	return cs.get();
}

void BakeConfiguration::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< BakeConfiguration >() >= 17);

	if (s.getVersion< BakeConfiguration >() >= 28)
		s >> Member< bool >(L"enableLightmaps", m_enableLightmaps);

	if (s.getVersion< BakeConfiguration >() >= 24)
	{
		s >> Member< uint32_t >(L"primarySampleCount", m_primarySampleCount, AttributeRange(0));
		s >> Member< uint32_t >(L"secondarySampleCount", m_secondarySampleCount, AttributeRange(0));
	}
	else
	{
		s >> Member< uint32_t >(L"sampleCount", m_primarySampleCount, AttributeRange(0));
		m_secondarySampleCount = m_primarySampleCount;
	}

	s >> Member< uint32_t >(L"shadowSampleCount", m_shadowSampleCount, AttributeRange(0));

	if (s.getVersion< BakeConfiguration >() < 22)
	{
		uint32_t irradianceSampleCount;
		s >> Member< uint32_t >(L"irradianceSampleCount", irradianceSampleCount, AttributeRange(0));
	}

	if (s.getVersion< BakeConfiguration >() >= 18)
		s >> Member< float >(L"maxPathDistance", m_maxPathDistance, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));

	s >> Member< float >(L"pointLightShadowRadius", m_pointLightShadowRadius, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"lumelDensity", m_lumelDensity, AttributeRange(0.0f));

	if (s.getVersion< BakeConfiguration >() >= 31)
		s >> Member< Vector4 >(L"irradianceGridDensity", m_irradianceGridDensity, AttributeRange(0.0f));
	else
	{
		float value;
		s >> Member< float >(L"irradianceGridDensity", value, AttributeRange(0.0f));
		m_irradianceGridDensity = Vector4(value, value, value, 0.0f);
	}

	s >> Member< int32_t >(L"minimumLightMapSize", m_minimumLightMapSize, AttributeRange(0));
	s >> Member< int32_t >(L"maximumLightMapSize", m_maximumLightMapSize, AttributeRange(0));

	if (s.getVersion< BakeConfiguration >() >= 1 && s.getVersion< BakeConfiguration >() < 19)
	{
		bool enableShadowFix;
		s >> Member< bool >(L"enableShadowFix", enableShadowFix);
	}

	s >> Member< bool >(L"enableDenoise", m_enableDenoise);

	if (s.getVersion< BakeConfiguration >() >= 12 && s.getVersion< BakeConfiguration >() < 21)
	{
		bool enableSeamFilter;
		s >> Member< bool >(L"enableSeamFilter", enableSeamFilter);
	}

	if (s.getVersion< BakeConfiguration >() >= 32)
		s >> Member< float >(L"analyticalLightAttenuation", m_analyticalLightAttenuation, AttributeRange(0.0f) | AttributeUnit(UnitType::Percent));

	if (s.getVersion< BakeConfiguration >() >= 20)
		s >> Member< float >(L"skyAttenuation", m_skyAttenuation, AttributeRange(0.0f) | AttributeUnit(UnitType::Percent));

	if (s.getVersion< BakeConfiguration >() >= 23)
		s >> Member< float >(L"ambientOcclusionFactor", m_ambientOcclusionFactor, AttributeRange(0.0f) | AttributeUnit(UnitType::Percent));

	if (s.getVersion< BakeConfiguration >() >= 25 && s.getVersion< BakeConfiguration >() < 29)
	{
		bool irradianceCache;
		s >> Member< bool >(L"irradianceCache", irradianceCache);
	}

	if (s.getVersion< BakeConfiguration >() >= 27 && s.getVersion< BakeConfiguration >() < 29)
	{
		float irradianceCacheMaxDistance;
		s >> Member< float >(L"irradianceCacheMaxDistance", irradianceCacheMaxDistance, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	}

	if (s.getVersion< BakeConfiguration >() >= 26 && s.getVersion< BakeConfiguration >() < 30)
	{
		bool enableDirectionalMaps;
		s >> Member< bool >(L"enableDirectionalMaps", enableDirectionalMaps);
	}
}

}
