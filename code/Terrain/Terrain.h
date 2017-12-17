/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_terrain_Terrain_H
#define traktor_terrain_Terrain_H

#include "Core/Object.h"
#include "Resource/Proxy.h"

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

/*! \brief Terrain runtime description.
 * \ingroup Terrain
 */
class T_DLLCLASS Terrain : public Object
{
	T_RTTI_CLASS;

public:
	struct Patch
	{
		float height[2];
		float error[3];
	};

	Terrain();

	/*! \brief Get amount of detail to skip.
	 *
	 * \return Level of detail skipped.
	 */
	uint32_t getDetailSkip() const { return m_detailSkip; }

	/*! \brief Get size of a single patch.
	 *
	 * \return Size of single patch.
	 */
	uint32_t getPatchDim() const { return m_patchDim; }

	/*! \brief Get heightfield resource.
	 *
	 * \return Heightfield resource.
	 */
	const resource::Proxy< hf::Heightfield >& getHeightfield() const { return m_heightfield; }

	/*! \brief Get terrain color map resource.
	 *
	 * \return Color map resource.
	 */
	const resource::Proxy< render::ISimpleTexture >& getColorMap() const { return m_colorMap; }

	/*! \brief Get terrain normal map resource.
	 *
	 * \return Normal map resource.
	 */
	const resource::Proxy< render::ISimpleTexture >& getNormalMap() const { return m_normalMap; }

	/*! \brief Get heightmap resource.
	 *
	 * \return Heightmap resource.
	 */
	const resource::Proxy< render::ISimpleTexture >& getHeightMap() const { return m_heightMap; }

	/*! \brief Get material splat map resource.
	 *
	 * \return Material splat map resource.
	 */
	const resource::Proxy< render::ISimpleTexture >& getSplatMap() const { return m_splatMap; }

	/*! \brief Get terrain cut map resource.
	 *
	 * \return Cut map resource.
	 */
	const resource::Proxy< render::ISimpleTexture >& getCutMap() const { return m_cutMap; }

	/*! \brief Get terrain material mask map resource.
	 *
	 * \return Material mask map resource.
	 */
	const resource::Proxy< render::ISimpleTexture >& getMaterialMap() const { return m_materialMap; }

	/*! \brief Get terrain coarse shader resource.
	 *
	 * \return Terrain coarse shader resource.
	 */
	const resource::Proxy< render::Shader >& getTerrainCoarseShader() const { return m_terrainCoarseShader; }

	/*! \brief Get terrain detail shader resource.
	 *
	 * \return Terrain detail shader resource.
	 */
	const resource::Proxy< render::Shader >& getTerrainDetailShader() const { return m_terrainDetailShader; }

	/*! \brief Get terrain surface shader resource.
	 *
	 * \return Terrain surface shader resource.
	 */
	const resource::Proxy< render::Shader >& getSurfaceShader() const { return m_surfaceShader; }

	/*! \brief Get vector of patch information.
	 *
	 * \return Information about each patch.
	 */
	const std::vector< Patch >& getPatches() const { return m_patches; }

	/*! \brief Get vector of patch information.
	 *
	 * \return Information about each patch.
	 */
	std::vector< Patch >& editPatches() { return m_patches; }

private:
	friend class TerrainFactory;
	friend class TerrainEditModifier;

	uint32_t m_detailSkip;
	uint32_t m_patchDim;
	resource::Proxy< hf::Heightfield > m_heightfield;
	resource::Proxy< render::ISimpleTexture > m_colorMap;
	resource::Proxy< render::ISimpleTexture > m_normalMap;
	resource::Proxy< render::ISimpleTexture > m_heightMap;
	resource::Proxy< render::ISimpleTexture > m_splatMap;
	resource::Proxy< render::ISimpleTexture > m_cutMap;
	resource::Proxy< render::ISimpleTexture > m_materialMap;
	resource::Proxy< render::Shader > m_terrainCoarseShader;
	resource::Proxy< render::Shader > m_terrainDetailShader;
	resource::Proxy< render::Shader > m_surfaceShader;
	std::vector< Patch > m_patches;
};

	}
}

#endif	// traktor_terrain_Terrain_H
