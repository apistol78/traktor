/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Math/Float.h"
#include "Heightfield/Heightfield.h"
#include "Terrain/TerrainUtilities.h"

namespace traktor::terrain
{

uint32_t patchToGrid(const hf::Heightfield* heightfield, uint32_t patchDim, uint32_t detailSkip, uint32_t patch)
{
	const uint32_t heightfieldSize = heightfield->getSize();
	const uint32_t patchCount = heightfieldSize / (patchDim * detailSkip);
	return (heightfieldSize * patch) / patchCount;
}

uint32_t gridToPatch(const hf::Heightfield* heightfield, uint32_t patchDim, uint32_t detailSkip, uint32_t grid)
{
	const uint32_t heightfieldSize = heightfield->getSize();
	const uint32_t patchCount = heightfieldSize / (patchDim * detailSkip);
	return (grid * patchCount) / heightfieldSize;
}

void calculatePatchMinMaxHeight(const hf::Heightfield* heightfield, uint32_t patchX, uint32_t patchZ, uint32_t patchDim, uint32_t detailSkip, float outHeights[2])
{
	const uint32_t heightfieldSize = heightfield->getSize();
	const uint32_t patchCount = heightfieldSize / (patchDim * detailSkip);

	const int32_t pminX = (heightfieldSize * patchX) / patchCount;
	const int32_t pminZ = (heightfieldSize * patchZ) / patchCount;
	const int32_t pmaxX = (heightfieldSize * (patchX + 1)) / patchCount;
	const int32_t pmaxZ = (heightfieldSize * (patchZ + 1)) / patchCount;

	float minHeight =  std::numeric_limits< float >::max();
	float maxHeight = -std::numeric_limits< float >::max();

	for (int32_t z = pminZ; z <= pmaxZ; ++z)
	{
		for (int32_t x = pminX; x <= pmaxX; ++x)
		{
			float height = heightfield->getGridHeightNearest(x, z);
			height = heightfield->unitToWorld(height);

			minHeight = min(minHeight, height);
			maxHeight = max(maxHeight, height);
		}
	}

	outHeights[0] = minHeight;
	outHeights[1] = maxHeight;
}

void calculatePatchErrorMetrics(const hf::Heightfield* heightfield, uint32_t lodCount, uint32_t patchX, uint32_t patchZ, uint32_t patchDim, uint32_t detailSkip, float* outErrors)
{
	const float c_errorBias = 0.25f;	// 25% average error, 75% max error

	const uint32_t heightfieldSize = heightfield->getSize();
	const uint32_t patchCount = heightfieldSize / (patchDim * detailSkip);

	const int32_t pminX = (heightfieldSize * patchX) / patchCount;
	const int32_t pminZ = (heightfieldSize * patchZ) / patchCount;

	for (uint32_t lod = 1; lod < lodCount; ++lod)
	{
		float maxError = 0.0f;
		float avgError = 0.0f;
		int32_t avgCount = 0;

		const uint32_t lodSkip = 1 << lod;
		for (uint32_t z = 0; z < patchDim; z += lodSkip)
		{
			for (uint32_t x = 0; x < patchDim; x += lodSkip)
			{
				const float fx0 = float(x) / (patchDim - 1);
				const float fz0 = float(z) / (patchDim - 1);
				const float fx1 = float(x + lodSkip) / (patchDim - 1);
				const float fz1 = float(z + lodSkip) / (patchDim - 1);

				const float gx0 = (fx0 * patchDim * detailSkip) + pminX;
				const float gz0 = (fz0 * patchDim * detailSkip) + pminZ;
				const float gx1 = (fx1 * patchDim * detailSkip) + pminX;
				const float gz1 = (fz1 * patchDim * detailSkip) + pminZ;

				const float h[] =
				{
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

						const float h = heightfield->getGridHeightBilinear(gx, gz);

						const float herr0 = abs(h - h0);
						const float herr1 = abs(h - h1);
						float herr = max(herr0, herr1);

						herr = heightfield->getWorldExtent().y() * herr;

						maxError = max(maxError, herr);
						avgError += herr; avgCount++;
					}
				}
			}
		}

		avgError /= float(avgCount);

		*outErrors++ = maxError * (1.0f - c_errorBias) + avgError * c_errorBias;
	}
}

}
