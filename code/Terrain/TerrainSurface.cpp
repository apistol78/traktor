#include "Terrain/TerrainSurface.h"
#include "Render/Shader.h"
#include "Render/ShaderGraph.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.terrain.TerrainSurface", TerrainSurface, Serializable)

bool TerrainSurface::serialize(Serializer& s)
{
	return s >> MemberStlVector< resource::Proxy< render::Shader >, resource::Member< render::Shader, render::ShaderGraph > >(L"layers", m_layers);
}

	}
}
