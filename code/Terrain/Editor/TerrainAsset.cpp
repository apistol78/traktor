#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/MaterialMask.h"
#include "Render/ISimpleTexture.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/Editor/TerrainAsset.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.TerrainAsset", 3, TerrainAsset, ISerializable)

TerrainAsset::TerrainAsset()
:	m_detailSkip(2)
,	m_patchDim(129)
{
}

bool TerrainAsset::serialize(ISerializer& s)
{
	if (s.getVersion() >= 2)
	{
		s >> Member< uint32_t >(L"detailSkip", m_detailSkip);
		s >> Member< uint32_t >(L"patchDim", m_patchDim);
	}

	s >> resource::Member< hf::Heightfield >(L"heightfield", m_heightfield);

	if (s.getVersion() >= 3)
		s >> resource::Member< render::ISimpleTexture >(L"splatMap", m_splatMap);

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
