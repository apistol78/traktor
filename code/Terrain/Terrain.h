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
	struct Patch
	{
		float height[2];
		float error[3];
	};

	Terrain();

	uint32_t getDetailSkip() const { return m_detailSkip; }

	uint32_t getPatchDim() const { return m_patchDim; }

	const resource::Proxy< hf::Heightfield >& getHeightfield() const { return m_heightfield; }

	const resource::Proxy< render::ISimpleTexture >& getNormalMap() const { return m_normalMap; }

	const resource::Proxy< render::ISimpleTexture >& getHeightMap() const { return m_heightMap; }

	const resource::Proxy< render::Shader >& getTerrainCoarseShader() const { return m_terrainCoarseShader; }

	const resource::Proxy< render::Shader >& getTerrainDetailShader() const { return m_terrainDetailShader; }

	const resource::Proxy< render::Shader >& getSurfaceShader() const { return m_surfaceShader; }

	const std::vector< Patch >& getPatches() const { return m_patches; }

private:
	friend class TerrainFactory;
	friend class TerrainEditModifier;

	uint32_t m_detailSkip;
	uint32_t m_patchDim;
	resource::Proxy< hf::Heightfield > m_heightfield;
	resource::Proxy< render::ISimpleTexture > m_normalMap;
	resource::Proxy< render::ISimpleTexture > m_heightMap;
	resource::Proxy< render::Shader > m_terrainCoarseShader;
	resource::Proxy< render::Shader > m_terrainDetailShader;
	resource::Proxy< render::Shader > m_surfaceShader;
	std::vector< Patch > m_patches;
};

	}
}

#endif	// traktor_terrain_Terrain_H
