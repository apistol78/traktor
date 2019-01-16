#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeHdr.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "World/Entity/SpotLightEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.SpotLightEntityData", 2, SpotLightEntityData, EntityData)

SpotLightEntityData::SpotLightEntityData()
:	m_color(1.0f, 1.0f, 1.0f, 0.0f)
,	m_range(0.0f)
,	m_radius(deg2rad(45.0f))
,	m_castShadow(false)
{
}

void SpotLightEntityData::serialize(ISerializer& s)
{
	EntityData::serialize(s);

	if (s.getVersion() >= 2)
		s >> Member< Vector4 >(L"color", m_color, AttributeHdr());
	else
	{
		Vector4 baseColor = Vector4::zero();
		Vector4 shadowColor = Vector4::zero();

		s >> Member< Vector4 >(L"sunColor", m_color);
		s >> Member< Vector4 >(L"baseColor", baseColor);
		s >> Member< Vector4 >(L"shadowColor", shadowColor);
	}

	s >> Member< float >(L"range", m_range);
	s >> Member< float >(L"radius", m_radius, AttributeRange(0.0f, PI));

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"castShadow", m_castShadow);
}

	}
}
