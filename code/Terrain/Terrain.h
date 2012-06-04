#ifndef traktor_terrain_Terrain_H
#define traktor_terrain_Terrain_H

#include "Core/Object.h"
#include "Resource/Proxy.h"

namespace traktor
{
	namespace hf
	{

class Heightfield;

	}

	namespace render
	{

class ISimpleTexture;
class Shader;

	}

	namespace terrain
	{

class Terrain : public Object
{
	T_RTTI_CLASS;

public:
	const resource::Proxy< hf::Heightfield >& getHeightfield() const { return m_heightfield; }

	const resource::Proxy< render::ISimpleTexture >& getNormalMap() const { return m_normalMap; }

	const resource::Proxy< render::ISimpleTexture >& getHeightMap() const { return m_heightMap; }

	const resource::Proxy< render::Shader >& getTerrainCoarseShader() const { return m_terrainCoarseShader; }

	const resource::Proxy< render::Shader >& getTerrainDetailShader() const { return m_terrainDetailShader; }

	const resource::Proxy< render::Shader >& getSurfaceShader() const { return m_surfaceShader; }

private:
	friend class TerrainFactory;

	resource::Proxy< hf::Heightfield > m_heightfield;
	resource::Proxy< render::ISimpleTexture > m_normalMap;
	resource::Proxy< render::ISimpleTexture > m_heightMap;
	resource::Proxy< render::Shader > m_terrainCoarseShader;
	resource::Proxy< render::Shader > m_terrainDetailShader;
	resource::Proxy< render::Shader > m_surfaceShader;
};

	}
}

#endif	// traktor_terrain_Terrain_H
