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

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.TerrainAsset", 0, TerrainAsset, ISerializable)

bool TerrainAsset::serialize(ISerializer& s)
{
	s >> resource::Member< hf::Heightfield >(L"heightfield", m_heightfield);
	s >> MemberStlVector< resource::Id< render::Shader >, resource::Member< render::Shader > >(L"surfaceLayers", m_surfaceLayers);
	return true;
}

	}
}
