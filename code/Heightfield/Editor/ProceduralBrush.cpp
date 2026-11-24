/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/ProceduralBrush.h"

#include "Core/Containers/AlignedVector.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Heightfield/Editor/ProceduralLayer.h"
#include "Heightfield/Heightfield.h"

#include <algorithm>

namespace traktor::hf
{
namespace
{

const int32_t c_rowsPerTask = 64;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.ProceduralBrush", ProceduralBrush, ISerializable)

void ProceduralBrush::apply(Heightfield* heightfield, const ProceduralLayer* layer) const
{
	const int32_t size = heightfield->getSize();
	const float spacing = (float)heightfield->getWorldExtent().x() / size;
	const float reach = getReach();

	// Rows are modified in parallel.
	AlignedVector< Job::task_t > tasks;
	for (int32_t z0 = 0; z0 < size; z0 += c_rowsPerTask)
		tasks.push_back([&, z0]() {
			AlignedVector< float > distances(size);
			AlignedVector< float > layerHeights(size);
			for (int32_t z = z0; z < std::min(z0 + c_rowsPerTask, size); ++z)
			{
				layer->evaluate(heightfield, z, reach, distances.ptr(), layerHeights.ptr());
				for (int32_t x = 0; x < size; ++x)
				{
					if (distances[x] <= -reach)
						continue;

					float worldX, worldZ;
					heightfield->gridToWorld(x, z, worldX, worldZ);
					const float height = heightfield->unitToWorld(heightfield->getGridHeightNearestUnsafe(x, z));
					heightfield->setGridHeight(x, z, heightfield->worldToUnit(sample(worldX, worldZ, spacing, height, distances[x], layerHeights[x])));
				}
			}
		});
	JobManager::getInstance().fork(tasks.c_ptr(), tasks.size());
}

float ProceduralBrush::getReach() const
{
	return 0.0f;
}

float ProceduralBrush::sample(float worldX, float worldZ, float spacing, float height, float distance, float layerHeight) const
{
	return height;
}

float ProceduralBrush::falloffWeight(float distance, float falloff)
{
	if (distance >= 0.0f)
		return 1.0f;
	if (distance <= -falloff)
		return 0.0f;
	const float t = 1.0f + distance / falloff;
	return t * t * (3.0f - 2.0f * t);
}

}
