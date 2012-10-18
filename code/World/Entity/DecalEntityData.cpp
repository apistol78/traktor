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

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.DecalEntityData", 0, DecalEntityData, EntityData)

DecalEntityData::DecalEntityData()
:	m_size(1.0f)
{
}

bool DecalEntityData::serialize(ISerializer& s)
{
	if (!EntityData::serialize(s))
		return false;

	s >> Member< float >(L"size", m_size, AttributeRange(0.0f));
	s >> resource::Member< render::Shader >(L"shader", m_shader);

	return true;
}

	}
}
