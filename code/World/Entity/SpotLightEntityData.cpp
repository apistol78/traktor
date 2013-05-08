#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "World/Entity/SpotLightEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.SpotLightEntityData", 1, SpotLightEntityData, EntityData)

SpotLightEntityData::SpotLightEntityData()
:	m_sunColor(1.0f, 1.0f, 1.0f, 0.0f)
,	m_baseColor(0.5f, 0.5f, 0.5f, 0.0f)
,	m_shadowColor(0.0f, 0.0f, 0.0f, 0.0f)
,	m_range(0.0f)
,	m_radius(deg2rad(45.0f))
,	m_castShadow(false)
{
}

void SpotLightEntityData::serialize(ISerializer& s)
{
	EntityData::serialize(s);

	s >> Member< Vector4 >(L"sunColor", m_sunColor);
	s >> Member< Vector4 >(L"baseColor", m_baseColor);
	s >> Member< Vector4 >(L"shadowColor", m_shadowColor);
	s >> Member< float >(L"range", m_range);
	s >> Member< float >(L"radius", m_radius, AttributeRange(0.0f, PI));

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"castShadow", m_castShadow);
}

	}
}
