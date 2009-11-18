#include "Terrain/TerrainEntityData.h"
#include "Terrain/Heightfield.h"
#include "Terrain/HeightfieldResource.h"
#include "Terrain/TerrainSurface.h"
#include "Render/Shader.h"
#include "Render/ShaderGraph.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.TerrainEntityData", 0, TerrainEntityData, world::EntityData)

TerrainEntityData::TerrainEntityData()
:	m_patchLodDistance(1500.0f)
,	m_surfaceLodDistance(100.0f)
{
}

bool TerrainEntityData::serialize(ISerializer& s)
{
	s >> resource::Member< Heightfield, HeightfieldResource >(L"heightfield", m_heightfield);
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
	s >> MemberRef< TerrainSurface >(L"surface", m_surface);
	s >> Member< float >(L"patchLodDistance", m_patchLodDistance);
	s >> Member< float >(L"surfaceLodDistance", m_surfaceLodDistance);
	return true;
}

	}
}
