#include "Core/Serialization/ISerializer.h"
#include "Resource/Member.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainEntityData.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.TerrainEntityData", 2, TerrainEntityData, world::EntityData)

TerrainEntityData::TerrainEntityData()
:	m_patchLodDistance(100.0f)
,	m_patchLodBias(0.0f)
,	m_patchLodExponent(1.0f)
,	m_surfaceLodDistance(100.0f)
,	m_surfaceLodBias(0.0f)
,	m_surfaceLodExponent(1.0f)
{
}

void TerrainEntityData::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 2);

	world::EntityData::serialize(s);

	s >> resource::Member< Terrain >(L"terrain", m_terrain);
	s >> Member< float >(L"patchLodDistance", m_patchLodDistance);
	s >> Member< float >(L"patchLodBias", m_patchLodBias);
	s >> Member< float >(L"patchLodExponent", m_patchLodExponent);
	s >> Member< float >(L"surfaceLodDistance", m_surfaceLodDistance);
	s >> Member< float >(L"surfaceLodBias", m_surfaceLodBias);
	s >> Member< float >(L"surfaceLodExponent", m_surfaceLodExponent);
}

	}
}
