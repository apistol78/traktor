#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "World/Entity/DecalEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.DecalEntityData", 2, DecalEntityData, EntityData)

DecalEntityData::DecalEntityData()
:	m_size(1.0f)
,	m_thickness(1.0f)
,	m_alpha(2.0f)
{
}

void DecalEntityData::serialize(ISerializer& s)
{
	EntityData::serialize(s);

	s >> Member< float >(L"size", m_size, AttributeRange(0.0f));

	if (s.getVersion() >= 2)
		s >> Member< float >(L"thickness", m_thickness, AttributeRange(0.0f));

	if (s.getVersion() >= 1)
		s >> Member< float >(L"alpha", m_alpha, AttributeRange(0.0f));

	s >> resource::Member< render::Shader >(L"shader", m_shader);
}

	}
}
