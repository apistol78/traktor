#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldResource.h"
#include "Render/Shader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Resource/Member.h"
#include "Terrain/TerrainEntityData.h"
#include "Terrain/TerrainSurface.h"

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
	if (!world::EntityData::serialize(s))
		return false;

	s >> resource::Member< hf::Heightfield, hf::HeightfieldResource >(L"heightfield", m_heightfield);
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
	s >> MemberRef< TerrainSurface >(L"surface", m_surface);
	s >> Member< float >(L"patchLodDistance", m_patchLodDistance);
	s >> Member< float >(L"surfaceLodDistance", m_surfaceLodDistance);

	return true;
}

	}
}
