#include "Core/Serialization/AttributeHdr.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "World/Entity/PointLightEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.PointLightEntityData", 3, PointLightEntityData, EntityData)

PointLightEntityData::PointLightEntityData()
:	m_color(1.0f, 1.0f, 1.0f, 0.0f)
,	m_range(0.0f)
,	m_randomFlickerAmount(0.0f)
,	m_randomFlickerFilter(0.5f)
{
}

void PointLightEntityData::serialize(ISerializer& s)
{
	EntityData::serialize(s);

	if (s.getVersion() >= 3)
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

	if (s.getVersion() == 1)
		s >> Member< float >(L"randomFlicker", m_randomFlickerAmount, AttributeRange(0.0f, 1.0f));
	else if (s.getVersion() >= 2)
	{
		s >> Member< float >(L"randomFlickerAmount", m_randomFlickerAmount, AttributeRange(0.0f, 1.0f));
		s >> Member< float >(L"randomFlickerFilter", m_randomFlickerFilter, AttributeRange(0.0f, 1.0f));
	}
}

	}
}
