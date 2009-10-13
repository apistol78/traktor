#include "Weather/Clouds/CloudParticleData.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.weather.CloudParticleData", CloudParticleData, Serializable)

CloudParticleData::CloudParticleData()
:	m_count(1000)
,	m_radiusMin(1.0f)
,	m_radiusRange(2.0f)
,	m_density(0.001f)
,	m_size(1.0f, 1.0f, 1.0f, 0.0f)
,	m_octaves(8)
,	m_persistence(0.8f)
,	m_noiseScale(1.0f)
{
}

bool CloudParticleData::serialize(Serializer& s)
{
	s >> Member< uint32_t >(L"count", m_count);
	s >> Member< float >(L"radiusMin", m_radiusMin);
	s >> Member< float >(L"radiusRange", m_radiusRange);
	s >> Member< float >(L"density", m_density);
	s >> Member< Vector4 >(L"size", m_size);
	s >> Member< int32_t >(L"octaves", m_octaves);
	s >> Member< float >(L"persistence", m_persistence);
	s >> Member< float >(L"noiseScale", m_noiseScale);
	return true;
}

	}
}
