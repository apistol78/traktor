#ifndef traktor_terrain_TerrainResource_H
#define traktor_terrain_TerrainResource_H

#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

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

/*! \brief Terrain persistent resource data.
 * \ingroup Terrain
 */
class T_DLLCLASS TerrainResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct Patch
	{
		float height[2];
		float error[3];

		void serialize(ISerializer& s);
	};

	TerrainResource();

	virtual void serialize(ISerializer& s);

	uint32_t getDetailSkip() const { return m_detailSkip; }

	uint32_t getPatchDim() const { return m_patchDim; }

	const resource::Id< hf::Heightfield >& getHeightfield() const { return m_heightfield; }

	const resource::Id< render::ISimpleTexture >& getColorMap() const { return m_colorMap; }

	const resource::Id< render::ISimpleTexture >& getNormalMap() const { return m_normalMap; }

	const resource::Id< render::ISimpleTexture >& getHeightMap() const { return m_heightMap; }

	const resource::Id< render::ISimpleTexture >& getSplatMap() const { return m_splatMap; }

	const resource::Id< render::ISimpleTexture >& getCutMap() const { return m_cutMap; }

	const resource::Id< render::Shader >& getTerrainCoarseShader() const { return m_terrainCoarseShader; }

	const resource::Id< render::Shader >& getTerrainDetailShader() const { return m_terrainDetailShader; }

	const resource::Id< render::Shader >& getSurfaceShader() const { return m_surfaceShader; }

	const std::vector< Patch >& getPatches() const { return m_patches; }

private:
	friend class TerrainPipeline;

	uint32_t m_detailSkip;
	uint32_t m_patchDim;
	resource::Id< hf::Heightfield > m_heightfield;
	resource::Id< render::ISimpleTexture > m_colorMap;
	resource::Id< render::ISimpleTexture > m_normalMap;
	resource::Id< render::ISimpleTexture > m_heightMap;
	resource::Id< render::ISimpleTexture > m_splatMap;
	resource::Id< render::ISimpleTexture > m_cutMap;
	resource::Id< render::Shader > m_terrainCoarseShader;
	resource::Id< render::Shader > m_terrainDetailShader;
	resource::Id< render::Shader > m_surfaceShader;
	std::vector< Patch > m_patches;
};

	}
}

#endif	// traktor_terrain_TerrainResource_H
