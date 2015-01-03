#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Heightfield/Heightfield.h"
#include "Render/ISimpleTexture.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/TerrainResource.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainResource", 6, TerrainResource, ISerializable)

TerrainResource::TerrainResource()
:	m_detailSkip(0)
,	m_patchDim(0)
{
}

void TerrainResource::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 6);
	s >> Member< uint32_t >(L"detailSkip", m_detailSkip);
	s >> Member< uint32_t >(L"patchDim", m_patchDim);
	s >> resource::Member< hf::Heightfield >(L"heightfield", m_heightfield);
	s >> resource::Member< render::ISimpleTexture >(L"colorMap", m_colorMap);
	s >> resource::Member< render::ISimpleTexture >(L"normalMap", m_normalMap);
	s >> resource::Member< render::ISimpleTexture >(L"heightMap", m_heightMap);
	s >> resource::Member< render::ISimpleTexture >(L"splatMap", m_splatMap);
	s >> resource::Member< render::ISimpleTexture >(L"cutMap", m_cutMap);
	s >> resource::Member< render::Shader >(L"terrainCoarseShader", m_terrainCoarseShader);
	s >> resource::Member< render::Shader >(L"terrainDetailShader", m_terrainDetailShader);
	s >> resource::Member< render::Shader >(L"surfaceShader", m_surfaceShader);
	s >> MemberStlVector< Patch, MemberComposite< Patch > >(L"patches", m_patches);
}

void TerrainResource::Patch::serialize(ISerializer& s)
{
	s >> MemberStaticArray< float, 2 >(L"height", height);
	s >> MemberStaticArray< float, 3 >(L"error", error);
}

	}
}
