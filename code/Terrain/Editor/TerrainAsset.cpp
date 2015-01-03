#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Heightfield/Heightfield.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/Editor/TerrainAsset.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.terrain.TerrainAsset", 6, TerrainAsset, ISerializable)

TerrainAsset::TerrainAsset()
:	m_detailSkip(2)
,	m_patchDim(129)
{
}

void TerrainAsset::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 6);

	s >> Member< uint32_t >(L"detailSkip", m_detailSkip);
	s >> Member< uint32_t >(L"patchDim", m_patchDim);
	s >> resource::Member< hf::Heightfield >(L"heightfield", m_heightfield);
	s >> resource::Member< render::Shader >(L"surfaceShader", m_surfaceShader);
}

	}
}
