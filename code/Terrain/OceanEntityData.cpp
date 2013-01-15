#include "Core/Serialization/ISerializer.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/OceanEntityData.h"
#include "Terrain/Terrain.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.OceanEntityData", 0, OceanEntityData, world::EntityData)

bool OceanEntityData::serialize(ISerializer& s)
{
	if (!world::EntityData::serialize(s))
		return false;

	s >> resource::Member< Terrain >(L"terrain", m_terrain);
	s >> resource::Member< render::Shader >(L"shaderComposite", m_shaderComposite);

	return true;
}

	}
}
