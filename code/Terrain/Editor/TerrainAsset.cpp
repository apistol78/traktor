#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/MaterialMask.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/Editor/TerrainAsset.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.TerrainAsset", 1, TerrainAsset, ISerializable)

bool TerrainAsset::serialize(ISerializer& s)
{
	s >> resource::Member< hf::Heightfield >(L"heightfield", m_heightfield);
	if (s.getVersion() >= 1)
		s >> resource::Member< render::Shader >(L"surfaceShader", m_surfaceShader);
	else
	{
		std::vector< resource::Id< render::Shader > > surfaceLayers;
		s >> MemberStlVector< resource::Id< render::Shader >, resource::Member< render::Shader > >(L"surfaceLayers", surfaceLayers);
		if (!surfaceLayers.empty())
			m_surfaceShader = surfaceLayers.front();
	}
	return true;
}

	}
}
