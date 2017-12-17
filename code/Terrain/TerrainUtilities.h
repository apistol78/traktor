#ifndef traktor_terrain_TerrainUtilities_H
#define traktor_terrain_TerrainUtilities_H

#include "Core/Config.h"

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

	namespace terrain
	{

/*! \brief Calculate grid coordinate from patch coordinate.
 * \ingroup Terrain
 */	
uint32_t T_DLLCLASS patchToGrid(const hf::Heightfield* heightfield, uint32_t patchDim, uint32_t detailSkip, uint32_t patch);
	
/*! \brief Calculate patch coordinate from grid coordinate.
 * \ingroup Terrain
 */	
uint32_t T_DLLCLASS gridToPatch(const hf::Heightfield* heightfield, uint32_t patchDim, uint32_t detailSkip, uint32_t grid);

/*! \brief Calculate min/max heights for patch.
 * \ingroup Terrain
 */	
void T_DLLCLASS calculatePatchMinMaxHeight(const hf::Heightfield* heightfield, uint32_t patchX, uint32_t patchZ, uint32_t patchDim, uint32_t detailSkip, float outHeights[2]);

/*! \brief Calculate error metrics for patch.
 * \ingroup Terrain
 */	
void T_DLLCLASS calculatePatchErrorMetrics(const hf::Heightfield* heightfield, uint32_t lodCount, uint32_t patchX, uint32_t patchZ, uint32_t patchDim, uint32_t detailSkip, float* outErrors);

	}
}

#endif	// traktor_terrain_TerrainUtilities_H
