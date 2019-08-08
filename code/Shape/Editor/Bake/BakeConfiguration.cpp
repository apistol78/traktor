#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Shape/Editor/Bake/BakeConfiguration.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.BakeConfiguration", 10, BakeConfiguration, ISerializable)

BakeConfiguration::BakeConfiguration()
:	m_seedGuid(Guid::create())
,	m_traceDirect(false)
,	m_traceIndirect(true)
,	m_indirectSampleCount(100)
,	m_shadowSampleCount(100)
,	m_pointLightShadowRadius(0.1f)
,	m_lumelDensity(64.0f)
,	m_minimumLightMapSize(128)
,	m_enableShadowFix(false)
,	m_enableDenoise(true)
,	m_clampShadowThreshold(0.01f)
{
}

void BakeConfiguration::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"seedGuid", m_seedGuid, AttributePrivate());
	s >> Member< bool >(L"traceDirect", m_traceDirect);
	s >> Member< bool >(L"traceIndirect", m_traceIndirect);

	if (s.getVersion() >= 7 && s.getVersion() < 10)
	{
		bool traceDebug;
		s >> Member< bool >(L"traceDebug", traceDebug);
	}

	s >> Member< uint32_t >(L"indirectSampleCount", m_indirectSampleCount, AttributeRange(0));
	s >> Member< uint32_t >(L"shadowSampleCount", m_shadowSampleCount, AttributeRange(0));
	s >> Member< float >(L"pointLightShadowRadius", m_pointLightShadowRadius, AttributeRange(0.0f));
	s >> Member< float >(L"lumelDensity", m_lumelDensity, AttributeRange(0.0f));

	if (s.getVersion() >= 5)
		s >> Member< int32_t >(L"minimumLightMapSize", m_minimumLightMapSize, AttributeRange(0));

	if (s.getVersion() >= 2 && s.getVersion() < 8)
	{
		bool enableAutoTexCoords;
		s >> Member< bool >(L"enableAutoTexCoords", enableAutoTexCoords);
	}

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"enableShadowFix", m_enableShadowFix);

	if (s.getVersion() >= 3 && s.getVersion() < 9)
	{
		bool enableDilate;
		s >> Member< bool >(L"enableDilate", enableDilate);
	}

	if (s.getVersion() >= 4)
		s >> Member< bool >(L"enableDenoise", m_enableDenoise);

	if (s.getVersion() >= 6)
		s >> Member< float >(L"clampShadowThreshold", m_clampShadowThreshold, AttributeRange(0.0f));
}

	}
}