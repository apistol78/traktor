#include "Core/Serialization/ISerializer.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/OceanEntityData.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.OceanEntityData", 0, OceanEntityData, world::EntityData)

void OceanEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);
	s >> resource::Member< render::Shader >(L"shaderComposite", m_shaderComposite);
}

	}
}
