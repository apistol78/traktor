#include "Core/Serialization/ISerializer.h"
#include "Heightfield/Heightfield.h"
#include "Render/ISimpleTexture.h"
#include "Render/Shader.h"
#include "Resource/Member.h"
#include "Terrain/TerrainResource.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.TerrainResource", 0, TerrainResource, ISerializable)

bool TerrainResource::serialize(ISerializer& s)
{
	s >> resource::Member< hf::Heightfield >(L"heightfield", m_heightfield);
	s >> resource::Member< render::ISimpleTexture >(L"normalMap", m_normalMap);
	s >> resource::Member< render::ISimpleTexture >(L"heightMap", m_heightMap);
	s >> resource::Member< render::Shader >(L"terrainCoarseShader", m_terrainCoarseShader);
	s >> resource::Member< render::Shader >(L"terrainDetailShader", m_terrainDetailShader);
	s >> resource::Member< render::Shader >(L"surfaceShader", m_surfaceShader);
	return true;
}

	}
}
