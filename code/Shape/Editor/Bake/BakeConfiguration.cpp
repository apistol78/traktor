#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.BakeConfiguration", 27, BakeConfiguration, ISerializable)

void BakeConfiguration::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< BakeConfiguration >() >= 17);

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
	s >> Member< float >(L"irradianceGridDensity", m_irradianceGridDensity, AttributeRange(0.0f));
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

	if (s.getVersion< BakeConfiguration >() >= 20)
		s >> Member< float >(L"skyAttenuation", m_skyAttenuation, AttributeRange(0.0f) | AttributeUnit(UnitType::Percent));

	if (s.getVersion< BakeConfiguration >() >= 23)
		s >> Member< float >(L"ambientOcclusionFactor", m_ambientOcclusionFactor, AttributeRange(0.0f) | AttributeUnit(UnitType::Percent));

	if (s.getVersion< BakeConfiguration >() >= 25)
		s >> Member< bool >(L"irradianceCache", m_irradianceCache);

	if (s.getVersion< BakeConfiguration >() >= 27)
		s >> Member< float >(L"irradianceCacheMaxDistance", m_irradianceCacheMaxDistance, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));

	if (s.getVersion< BakeConfiguration >() >= 26)
		s >> Member< bool >(L"enableDirectionalMaps", m_enableDirectionalMaps);
}

	}
}