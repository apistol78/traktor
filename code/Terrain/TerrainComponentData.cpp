#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Resource/Member.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponentData.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.TerrainComponentData", 0, TerrainComponentData, world::IEntityComponentData)

void TerrainComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void TerrainComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< Terrain >(L"terrain", m_terrain);
	s >> Member< float >(L"patchLodDistance", m_patchLodDistance, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"patchLodBias", m_patchLodBias);
	s >> Member< float >(L"patchLodExponent", m_patchLodExponent);
	s >> Member< float >(L"surfaceLodDistance", m_surfaceLodDistance, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"surfaceLodBias", m_surfaceLodBias);
	s >> Member< float >(L"surfaceLodExponent", m_surfaceLodExponent);
}

}
