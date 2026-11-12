/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::hf
{

class Heightfield;

}

namespace traktor::terrain
{

/*! Calculate min/max world heights of patch covering grid gridX to gridX + gridSize.
 * \ingroup Terrain
 */
void T_DLLCLASS calculatePatchMinMaxHeight(const hf::Heightfield* heightfield, float gridX, float gridZ, float gridSize, float outHeights[2]);

/*! Calculate world height error of each lod, 1 to lodCount - 1, of patch covering grid gridX to gridX + gridSize with patchDim vertices along each side.
 * \ingroup Terrain
 */
void T_DLLCLASS calculatePatchErrorMetrics(const hf::Heightfield* heightfield, uint32_t lodCount, float gridX, float gridZ, float gridSize, uint32_t patchDim, float* outErrors);

}
