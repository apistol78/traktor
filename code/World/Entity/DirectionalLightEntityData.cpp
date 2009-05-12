#include "World/Entity/DirectionalLightEntityData.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.DirectionalLightEntityData", DirectionalLightEntityData, SpatialEntityData)

DirectionalLightEntityData::DirectionalLightEntityData()
:	m_sunColor(1.0f, 1.0f, 1.0f, 0.0f)
,	m_baseColor(0.5f, 0.5f, 0.5f, 0.0f)
,	m_shadowColor(0.0f, 0.0f, 0.0f, 0.0f)
{
}

bool DirectionalLightEntityData::serialize(Serializer& s)
{
	if (!SpatialEntityData::serialize(s))
		return false;

	s >> Member< Vector4 >(L"sunColor", m_sunColor);
	s >> Member< Vector4 >(L"baseColor", m_baseColor);
	s >> Member< Vector4 >(L"shadowColor", m_shadowColor);

	return true;
}

	}
}
