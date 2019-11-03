#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.BakeConfiguration", 17, BakeConfiguration, ISerializable)

BakeConfiguration::BakeConfiguration()
:	m_sampleCount(100)
,	m_shadowSampleCount(100)
,	m_irradianceSampleCount(100)
,	m_pointLightShadowRadius(0.1f)
,	m_lumelDensity(32.0f)
,	m_irradianceGridDensity(1.0f)
,	m_minimumLightMapSize(128)
,	m_maximumLightMapSize(1024)
,	m_enableShadowFix(true)
,	m_enableDenoise(true)
,	m_enableSeamFilter(true)
{
}

void BakeConfiguration::serialize(ISerializer& s)
{
	if (s.getVersion< BakeConfiguration >() < 13)
	{
		Guid seedGuid;
		s >> Member< Guid >(L"seedGuid", seedGuid, AttributePrivate());
	}

	if (s.getVersion< BakeConfiguration >() < 15)
	{
		bool traceDirect, traceIndirect, traceIrradiance;

		s >> Member< bool >(L"traceDirect", traceDirect);
		s >> Member< bool >(L"traceIndirect", traceIndirect);

		if (s.getVersion< BakeConfiguration >() >= 11)
			s >> Member< bool >(L"traceIrradiance", traceIrradiance);
	}

	if (s.getVersion< BakeConfiguration >() >= 7 && s.getVersion< BakeConfiguration >() < 10)
	{
		bool traceDebug;
		s >> Member< bool >(L"traceDebug", traceDebug);
	}

	if (s.getVersion< BakeConfiguration >() >= 17)
		s >> Member< uint32_t >(L"sampleCount", m_sampleCount, AttributeRange(0));
	else
		s >> Member< uint32_t >(L"indirectSampleCount", m_sampleCount, AttributeRange(0));

	s >> Member< uint32_t >(L"shadowSampleCount", m_shadowSampleCount, AttributeRange(0));

	if (s.getVersion< BakeConfiguration >() >= 11)
		s >> Member< uint32_t >(L"irradianceSampleCount", m_irradianceSampleCount, AttributeRange(0));

	s >> Member< float >(L"pointLightShadowRadius", m_pointLightShadowRadius, AttributeRange(0.0f));
	s >> Member< float >(L"lumelDensity", m_lumelDensity, AttributeRange(0.0f));

	if (s.getVersion< BakeConfiguration >() >= 16)
		s >> Member< float >(L"irradianceGridDensity", m_irradianceGridDensity, AttributeRange(0.0f));

	if (s.getVersion< BakeConfiguration >() >= 5)
		s >> Member< int32_t >(L"minimumLightMapSize", m_minimumLightMapSize, AttributeRange(0));
	if (s.getVersion< BakeConfiguration >() >= 14)
		s >> Member< int32_t >(L"maximumLightMapSize", m_maximumLightMapSize, AttributeRange(0));

	if (s.getVersion< BakeConfiguration >() >= 2 && s.getVersion< BakeConfiguration >() < 8)
	{
		bool enableAutoTexCoords;
		s >> Member< bool >(L"enableAutoTexCoords", enableAutoTexCoords);
	}

	if (s.getVersion< BakeConfiguration >() >= 1)
		s >> Member< bool >(L"enableShadowFix", m_enableShadowFix);

	if (s.getVersion< BakeConfiguration >() >= 3 && s.getVersion< BakeConfiguration >() < 9)
	{
		bool enableDilate;
		s >> Member< bool >(L"enableDilate", enableDilate);
	}

	if (s.getVersion< BakeConfiguration >() >= 4)
		s >> Member< bool >(L"enableDenoise", m_enableDenoise);

	if (s.getVersion< BakeConfiguration >() >= 12)
		s >> Member< bool >(L"enableSeamFilter", m_enableSeamFilter);

	if (s.getVersion< BakeConfiguration >() >= 6 && s.getVersion< BakeConfiguration >() < 12)
	{
		float clampShadowThreshold;
		s >> Member< float >(L"clampShadowThreshold", clampShadowThreshold, AttributeRange(0.0f));
	}
}

	}
}