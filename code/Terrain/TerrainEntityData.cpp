#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Resource/Member.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainEntityData.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.TerrainEntityData", 1, TerrainEntityData, world::EntityData)

TerrainEntityData::TerrainEntityData()
:	m_patchLodDistance(100.0f)
,	m_patchLodBias(0.0f)
,	m_patchLodExponent(1.0f)
,	m_surfaceLodDistance(100.0f)
,	m_surfaceLodBias(0.0f)
,	m_surfaceLodExponent(1.0f)
,	m_visualizeMode(VmDefault)
{
}

void TerrainEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);

	s >> resource::Member< Terrain >(L"terrain", m_terrain);

	if (s.getVersion() < 1)
	{
		uint32_t detailSkip = 0, patchDim = 0;
		s >> Member< uint32_t >(L"detailSkip", detailSkip);
		s >> Member< uint32_t >(L"patchDim", patchDim);
	}

	s >> Member< float >(L"patchLodDistance", m_patchLodDistance);
	s >> Member< float >(L"patchLodBias", m_patchLodBias);
	s >> Member< float >(L"patchLodExponent", m_patchLodExponent);
	s >> Member< float >(L"surfaceLodDistance", m_surfaceLodDistance);
	s >> Member< float >(L"surfaceLodBias", m_surfaceLodBias);
	s >> Member< float >(L"surfaceLodExponent", m_surfaceLodExponent);

	const MemberEnum< VisualizeMode >::Key c_VisualizeMode_Keys[] =
	{
		{ L"VmDefault", VmDefault },
		{ L"VmSurfaceLod", VmSurfaceLod },
		{ L"VmPatchLod", VmPatchLod },
		{ 0 }
	};
	s >> MemberEnum< VisualizeMode >(L"visualizeMode", m_visualizeMode, c_VisualizeMode_Keys);
}

	}
}
