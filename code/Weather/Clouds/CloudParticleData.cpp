/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Weather/Clouds/CloudParticleData.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.weather.CloudParticleData", 0, CloudParticleData, ISerializable)

CloudParticleData::CloudParticleData()
:	m_count(1000)
,	m_radiusMin(1.0f)
,	m_radiusRange(2.0f)
,	m_density(10.0f)
,	m_haloColor(128, 128, 128)
,	m_skyColor(255, 255, 255)
,	m_groundColor(0, 0, 0)
,	m_sunInfluence(1.0f)
,	m_size(1.0f, 1.0f, 1.0f, 0.0f)
,	m_octaves(8)
,	m_persistence(0.8f)
,	m_noiseScale(1.0f)
{
}

void CloudParticleData::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"count", m_count);
	s >> Member< float >(L"radiusMin", m_radiusMin, AttributeRange(0.0f));
	s >> Member< float >(L"radiusRange", m_radiusRange, AttributeRange(0.0f));
	s >> Member< float >(L"density", m_density, AttributeRange(0.0f));
	s >> Member< Color4ub >(L"haloColor", m_haloColor);
	s >> Member< Color4ub >(L"skyColor", m_skyColor);
	s >> Member< Color4ub >(L"groundColor", m_groundColor);
	s >> Member< float >(L"sunInfluence", m_sunInfluence, AttributeRange(0.0f, 1.0f));
	s >> Member< Vector4 >(L"size", m_size, AttributeDirection());
	s >> Member< int32_t >(L"octaves", m_octaves);
	s >> Member< float >(L"persistence", m_persistence);
	s >> Member< float >(L"noiseScale", m_noiseScale);
}

	}
}
