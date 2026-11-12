/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Terrain/TerrainUtilities.h"

#include "Core/Math/Float.h"
#include "Heightfield/Heightfield.h"

#include <cmath>
#include <limits>

namespace traktor::terrain
{

void calculatePatchMinMaxHeight(const hf::Heightfield* heightfield, float gridX, float gridZ, float gridSize, float outHeights[2])
{
	const int32_t mnx = (int32_t)std::floor(gridX);
	const int32_t mnz = (int32_t)std::floor(gridZ);
	const int32_t mxx = (int32_t)std::ceil(gridX + gridSize);
	const int32_t mxz = (int32_t)std::ceil(gridZ + gridSize);

	float minHeight = std::numeric_limits< float >::max();
	float maxHeight = -std::numeric_limits< float >::max();

	for (int32_t z = mnz; z <= mxz; ++z)
	{
		for (int32_t x = mnx; x <= mxx; ++x)
		{
			const float height = heightfield->unitToWorld(heightfield->getGridHeightNearest(x, z));
			minHeight = min(minHeight, height);
			maxHeight = max(maxHeight, height);
		}
	}

	outHeights[0] = minHeight;
	outHeights[1] = maxHeight;
}

void calculatePatchErrorMetrics(const hf::Heightfield* heightfield, uint32_t lodCount, float gridX, float gridZ, float gridSize, uint32_t patchDim, float* outErrors)
{
	const float c_errorBias = 0.25f; // 25% average error, 75% max error
	const float vertexSize = gridSize / (patchDim - 1);

	for (uint32_t lod = 1; lod < lodCount; ++lod)
	{
		float maxError = 0.0f;
		float avgError = 0.0f;
		int32_t avgCount = 0;

		const uint32_t lodSkip = 1 << lod;
		for (uint32_t z = 0; z < patchDim - 1; z += lodSkip)
		{
			for (uint32_t x = 0; x < patchDim - 1; x += lodSkip)
			{
				const float gx0 = gridX + x * vertexSize;
				const float gz0 = gridZ + z * vertexSize;
				const float gx1 = gridX + (x + lodSkip) * vertexSize;
				const float gz1 = gridZ + (z + lodSkip) * vertexSize;

				const float h[] = {
					heightfield->getGridHeightBilinear(gx0, gz0),
					heightfield->getGridHeightBilinear(gx1, gz0),
					heightfield->getGridHeightBilinear(gx0, gz1),
					heightfield->getGridHeightBilinear(gx1, gz1)
				};

				for (uint32_t lz = 0; lz <= lodSkip; ++lz)
				{
					for (uint32_t lx = 0; lx <= lodSkip; ++lx)
					{
						const float fx = float(lx) / lodSkip;
						const float fz = float(lz) / lodSkip;

						const float gx = lerp(gx0, gx1, fx);
						const float gz = lerp(gz0, gz1, fz);

						const float ht = lerp(h[0], h[1], fx);
						const float hb = lerp(h[2], h[3], fx);
						const float h0 = lerp(ht, hb, fz);

						const float hl = lerp(h[0], h[2], fz);
						const float hr = lerp(h[1], h[3], fz);
						const float h1 = lerp(hl, hr, fx);

						const float hc = heightfield->getGridHeightBilinear(gx, gz);
						const float herr = max(abs(hc - h0), abs(hc - h1)) * heightfield->getWorldExtent().y();

						maxError = max(maxError, herr);
						avgError += herr;
						avgCount++;
					}
				}
			}
		}

		avgError /= float(avgCount);
		*outErrors++ = maxError * (1.0f - c_errorBias) + avgError * c_errorBias;
	}
}

}
