#include "World/Entity/DirectionalLightEntityData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.DirectionalLightEntityData", 1, DirectionalLightEntityData, EntityData)

DirectionalLightEntityData::DirectionalLightEntityData()
:	m_sunColor(1.0f, 1.0f, 1.0f, 0.0f)
,	m_baseColor(0.5f, 0.5f, 0.5f, 0.0f)
,	m_shadowColor(0.0f, 0.0f, 0.0f, 0.0f)
,	m_castShadow(true)
{
}

void DirectionalLightEntityData::serialize(ISerializer& s)
{
	EntityData::serialize(s);

	s >> Member< Vector4 >(L"sunColor", m_sunColor);
	s >> Member< Vector4 >(L"baseColor", m_baseColor);
	s >> Member< Vector4 >(L"shadowColor", m_shadowColor);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"castShadow", m_castShadow);
}

	}
}
