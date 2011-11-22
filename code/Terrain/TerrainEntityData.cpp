#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRef.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldResource.h"
#include "Heightfield/MaterialMask.h"
#include "Heightfield/MaterialMaskResource.h"
#include "Resource/Member.h"
#include "Terrain/TerrainEntityData.h"
#include "Terrain/TerrainSurface.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.TerrainEntityData", 1, TerrainEntityData, world::EntityData)

TerrainEntityData::TerrainEntityData()
:	m_patchLodDistance(100.0f)
,	m_surfaceLodDistance(100.0f)
,	m_visualizeMode(VmDefault)
{
}

bool TerrainEntityData::serialize(ISerializer& s)
{
	if (!world::EntityData::serialize(s))
		return false;

	s >> resource::Member< hf::Heightfield, hf::HeightfieldResource >(L"heightfield", m_heightfield);
	s >> resource::Member< hf::MaterialMask, hf::MaterialMaskResource >(L"materialMask", m_materialMask);
	s >> MemberRef< TerrainSurface >(L"surface", m_surface);
	s >> Member< float >(L"patchLodDistance", m_patchLodDistance);
	s >> Member< float >(L"surfaceLodDistance", m_surfaceLodDistance);

	if (s.getVersion() >= 1)
	{
		const MemberEnum< VisualizeMode >::Key c_VisualizeMode_Keys[] =
		{
			{ L"VmDefault", VmDefault },
			{ L"VmSurfaceLod", VmSurfaceLod },
			{ L"VmPatchLod", VmPatchLod },
			{ 0 }
		};
		s >> MemberEnum< VisualizeMode >(L"visualizeMode", m_visualizeMode, c_VisualizeMode_Keys);
	}

	return true;
}

	}
}
