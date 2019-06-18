#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Illuminate/Editor/IlluminateConfiguration.h"

namespace traktor
{
	namespace illuminate
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.illuminate.IlluminateConfiguration", 6, IlluminateConfiguration, ISerializable)

IlluminateConfiguration::IlluminateConfiguration()
:	m_seedGuid(Guid::create())
,	m_traceDirect(false)
,	m_traceIndirect(true)
,	m_indirectSampleCount(150)
,	m_shadowSampleCount(20)
,	m_pointLightShadowRadius(0.2f)
,	m_lumelDensity(32.0f)
,	m_minimumLightMapSize(16)
,	m_enableAutoTexCoords(true)
,	m_enableShadowFix(true)
,	m_enableDilate(true)
,	m_enableDenoise(true)
,	m_clampShadowThreshold(0.01f)
{
}

void IlluminateConfiguration::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"seedGuid", m_seedGuid, AttributePrivate());
	s >> Member< bool >(L"traceDirect", m_traceDirect);
	s >> Member< bool >(L"traceIndirect", m_traceIndirect);
	s >> Member< uint32_t >(L"indirectSampleCount", m_indirectSampleCount, AttributeRange(0));
	s >> Member< uint32_t >(L"shadowSampleCount", m_shadowSampleCount, AttributeRange(0));
	s >> Member< float >(L"pointLightShadowRadius", m_pointLightShadowRadius, AttributeRange(0.0f));
	s >> Member< float >(L"lumelDensity", m_lumelDensity, AttributeRange(0.0f));

	if (s.getVersion() >= 5)
		s >> Member< int32_t >(L"minimumLightMapSize", m_minimumLightMapSize, AttributeRange(0));

	if (s.getVersion() >= 2)
		s >> Member< bool >(L"enableAutoTexCoords", m_enableAutoTexCoords);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"enableShadowFix", m_enableShadowFix);

	if (s.getVersion() >= 3)
		s >> Member< bool >(L"enableDilate", m_enableDilate);

	if (s.getVersion() >= 4)
		s >> Member< bool >(L"enableDenoise", m_enableDenoise);

	if (s.getVersion() >= 6)
		s >> Member< float >(L"clampShadowThreshold", m_clampShadowThreshold, AttributeRange(0.0f));
}

	}
}