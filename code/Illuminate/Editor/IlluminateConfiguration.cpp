#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Illuminate/Editor/IlluminateConfiguration.h"

namespace traktor
{
	namespace illuminate
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.illuminate.IlluminateConfiguration", 0, IlluminateConfiguration, ISerializable)

IlluminateConfiguration::IlluminateConfiguration()
:	m_seedGuid(Guid::create())
,	m_traceDirect(true)
,	m_traceIndirect(true)
,	m_traceOcclusion(true)
,	m_irradianceSampleCount(64)
,	m_shadowSampleCount(32)
,	m_occlusionSampleCount(128)
,	m_pointLightShadowRadius(1.5f)
,	m_lumelDensity(4.0f)
{
}

void IlluminateConfiguration::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"seedGuid", m_seedGuid, AttributePrivate());
	s >> Member< bool >(L"traceDirect", m_traceDirect);
	s >> Member< bool >(L"traceIndirect", m_traceIndirect);
	s >> Member< bool >(L"traceOcclusion", m_traceOcclusion);
	s >> Member< uint32_t >(L"irradianceSampleCount", m_irradianceSampleCount);
	s >> Member< uint32_t >(L"shadowSampleCount", m_shadowSampleCount);
	s >> Member< uint32_t >(L"occlusionSampleCount", m_occlusionSampleCount);
	s >> Member< float >(L"pointLightShadowRadius", m_pointLightShadowRadius);
	s >> Member< float >(L"lumelDensity", m_lumelDensity);
}

	}
}