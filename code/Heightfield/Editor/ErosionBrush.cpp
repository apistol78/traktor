/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/ErosionBrush.h"

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/MathUtils.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Heightfield/Editor/ErosionFilter.h"
#include "Heightfield/Editor/ProceduralLayer.h"
#include "Heightfield/Heightfield.h"

#include <algorithm>
#include <cmath>

namespace traktor::hf
{
namespace
{

const int32_t c_rowsPerTask = 64;

//! Catmull-Rom spline between p1 and p2.
float catmullRom(float p0, float p1, float p2, float p3, float t)
{
	return p1 + 0.5f * t * (p2 - p0 + t * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3 + t * (3.0f * (p1 - p2) + p3 - p0)));
}

//! Bicubic Catmull-Rom sample of grid, clamped at edges.
float sampleCubic(const AlignedVector< float >& grid, int32_t size, float gridX, float gridZ)
{
	const int32_t ix = (int32_t)std::floor(gridX);
	const int32_t iz = (int32_t)std::floor(gridZ);

	float rows[4];
	for (int32_t j = 0; j < 4; ++j)
	{
		const float* row = grid.c_ptr() + (size_t)clamp(iz + j - 1, 0, size - 1) * size;
		rows[j] = catmullRom(row[clamp(ix - 1, 0, size - 1)], row[clamp(ix, 0, size - 1)], row[clamp(ix + 1, 0, size - 1)], row[clamp(ix + 2, 0, size - 1)], gridX - ix);
	}
	return catmullRom(rows[0], rows[1], rows[2], rows[3], gridZ - iz);
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.ErosionBrush", 0, ErosionBrush, ProceduralBrush)

void ErosionBrush::apply(Heightfield* heightfield, const ProceduralLayer* layer) const
{
	const int32_t size = heightfield->getSize();
	const Vector4& worldExtent = heightfield->getWorldExtent();
	const int32_t erodeSize = (int32_t)clamp((float)worldExtent.x() / m_resolution, 2.0f, (float)size);

	// Erode a copy of heightfield at erosion resolution.
	Ref< Heightfield > eroded = new Heightfield(erodeSize, worldExtent, heightfield->getWorldCenter());
	for (int32_t z = 0; z < erodeSize; ++z)
		for (int32_t x = 0; x < erodeSize; ++x)
		{
			float worldX, worldZ;
			eroded->gridToWorld(x, z, worldX, worldZ);
			eroded->setGridHeight(x, z, eroded->worldToUnit(heightfield->getWorldHeight(worldX, worldZ)));
		}

	const height_t* erodedHeights = eroded->getHeights();
	const AlignedVector< height_t > original(erodedHeights, erodedHeights + (size_t)erodeSize * erodeSize);
	ErosionFilter(m_iterations).apply(eroded);

	AlignedVector< float > difference(original.size());
	for (size_t i = 0; i < original.size(); ++i)
		difference[i] = ((float)erodedHeights[i] - (float)original[i]) * (float)worldExtent.y() / 65535.0f;

	// Eroded difference is smoothly upsampled and added to heightfield by layer, rows in parallel.
	AlignedVector< Job::task_t > tasks;
	for (int32_t z0 = 0; z0 < size; z0 += c_rowsPerTask)
		tasks.push_back([&, z0]() {
			AlignedVector< float > distances(size);
			AlignedVector< float > layerHeights(size);
			for (int32_t z = z0; z < std::min(z0 + c_rowsPerTask, size); ++z)
			{
				layer->evaluate(heightfield, z, m_falloff, distances.ptr(), layerHeights.ptr());
				for (int32_t x = 0; x < size; ++x)
				{
					if (distances[x] <= -m_falloff)
						continue;

					float worldX, worldZ;
					heightfield->gridToWorld(x, z, worldX, worldZ);
					const float height = heightfield->unitToWorld(heightfield->getGridHeightNearestUnsafe(x, z));
					float gridX, gridZ;
					eroded->worldToGrid(worldX, worldZ, gridX, gridZ);
					const float delta = sampleCubic(difference, erodeSize, gridX, gridZ);
					heightfield->setGridHeight(x, z, heightfield->worldToUnit(height + delta * falloffWeight(distances[x], m_falloff)));
				}
			}
		});
	JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());
}

void ErosionBrush::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"iterations", m_iterations, AttributeRange(0));
	s >> Member< float >(L"resolution", m_resolution, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"falloff", m_falloff, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
}

}
