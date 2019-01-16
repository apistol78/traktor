#include "Core/Serialization/AttributeHdr.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/ITexture.h"
#include "Resource/Member.h"
#include "World/Entity/DirectionalLightEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.DirectionalLightEntityData", 3, DirectionalLightEntityData, EntityData)

DirectionalLightEntityData::DirectionalLightEntityData()
:	m_color(1.0f, 1.0f, 1.0f, 0.0f)
,	m_castShadow(true)
{
}

void DirectionalLightEntityData::serialize(ISerializer& s)
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

	if (s.getVersion() >= 2)
		s >> resource::Member< render::ITexture >(L"cloudShadowTexture", m_cloudShadowTexture);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"castShadow", m_castShadow);
}

	}
}
