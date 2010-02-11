#include "Terrain/TerrainSurface.h"
#include "Render/Shader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainSurface", 0, TerrainSurface, ISerializable)

bool TerrainSurface::serialize(ISerializer& s)
{
	return s >> MemberStlVector< resource::Proxy< render::Shader >, resource::Member< render::Shader, render::ShaderGraph > >(L"layers", m_layers);
}

	}
}
