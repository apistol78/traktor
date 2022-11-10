/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cmath>
#include <limits>
#include "Core/Math/Aabb3.h"
#include "Core/Math/Winding3.h"
#include "Heightfield/Heightfield.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.Heightfield", Heightfield, Object)

Heightfield::Heightfield(
	int32_t size,
	const Vector4& worldExtent
)
:	m_size(size)
,	m_worldExtent(worldExtent)
{
	m_heights.reset(new height_t [m_size * m_size]);
	m_cuts.reset(new uint8_t [(m_size * m_size) / 8]);
	m_attributes.reset(new uint8_t [m_size * m_size]);
	m_worldExtent.storeUnaligned(m_worldExtentFloats);
}

void Heightfield::setGridHeight(int32_t gridX, int32_t gridZ, float unitY)
{
	if (gridX < 0 || gridX >= (int32_t)m_size)
		return;
	if (gridZ < 0 || gridZ >= (int32_t)m_size)
		return;
	m_heights[gridX + gridZ * m_size] = height_t(clamp(unitY, 0.0f, 1.0f) * 65535.0f);
}

void Heightfield::setGridCut(int32_t gridX, int32_t gridZ, bool cut)
{
	if (gridX < 0 || gridX >= (int32_t)m_size)
		return;
	if (gridZ < 0 || gridZ >= (int32_t)m_size)
		return;

	int32_t offset = gridX + gridZ * m_size;
	if (cut)
		m_cuts[offset / 8] |= (1 << (offset & 7));
	else
		m_cuts[offset / 8] &= ~(1 << (offset & 7));
}

void Heightfield::setGridAttribute(int32_t gridX, int32_t gridZ, uint8_t attribute)
{
	if (gridX < 0 || gridX >= (int32_t)m_size)
		return;
	if (gridZ < 0 || gridZ >= (int32_t)m_size)
		return;

	int32_t offset = gridX + gridZ * m_size;
	m_attributes[offset] = attribute;
}

float Heightfield::getGridHeightNearest(int32_t gridX, int32_t gridZ) const
{
	if (gridX < 0)
		gridX = 0;
	else if (gridX >= (int32_t)m_size)
		gridX = (int32_t)m_size - 1;

	if (gridZ < 0)
		gridZ = 0;
	else if (gridZ >= (int32_t)m_size)
		gridZ = (int32_t)m_size - 1;

	return m_heights[gridX + gridZ * m_size] / 65535.0f;
}

float Heightfield::getGridHeightBilinear(float gridX, float gridZ) const
{
	int32_t igridX = (int32_t)gridX;
	int32_t igridZ = (int32_t)gridZ;

	if (igridX < 0)
		igridX = 0;
	else if (igridX >= (int32_t)m_size - 1)
		igridX = (int32_t)m_size - 2;

	if (igridZ < 0)
		igridZ = 0;
	else if (igridZ >= (int32_t)m_size - 1)
		igridZ = (int32_t)m_size - 2;

	int32_t offset = igridX + igridZ * m_size;

	height_t hts[] =
	{
		m_heights[offset],
		m_heights[offset + 1],
		m_heights[offset + m_size],
		m_heights[offset + 1 + m_size]
	};

	float fgridX = gridX - igridX;
	float fgridZ = gridZ - igridZ;

#if 0
/*
0,1(2)  1,1(3)
   +----+
   |\   |
   | \  |
   |  \ |
   |   \|
   +----+
0,0(0)  1,0(1)
*/
	float k = 1.0f - (fgridX + fgridZ);
	if (k >= 0.0f)
	{
		// Left
		hts[3] = hts[2] + (hts[1] - hts[0]);
	}
	else
	{
		// Right
		hts[0] = hts[1] + (hts[2] - hts[3]);
	}
#else
/*
0,1(2)  1,1(3)
   +----+
   |   /|
   |  / |
   | /  |
   |/   |
   +----+
0,0(0)  1,0(1)
*/
	float k = fgridX - fgridZ;
	if (k < 0.0f)
	{
		// Left
		hts[1] = hts[0] + (hts[3] - hts[2]);
	}
	else
	{
		// Right
		hts[2] = hts[3] + (hts[0] - hts[1]);
	}
#endif

	float hl = hts[0] + (hts[2] - hts[0]) * fgridZ;
	float hr = hts[1] + (hts[3] - hts[1]) * fgridZ;

	return (hl + (hr - hl) * fgridX) / 65535.0f;
}

float Heightfield::getWorldHeight(float worldX, float worldZ) const
{
	float gridX, gridZ;
	worldToGrid(worldX, worldZ, gridX, gridZ);
	float gridY = getGridHeightBilinear(gridX, gridZ);
	return -m_worldExtentFloats[1] * 0.5f + gridY * m_worldExtentFloats[1];
}

bool Heightfield::getGridCut(int32_t gridX, int32_t gridZ) const
{
	if (gridX < 0)
		gridX = 0;
	else if (gridX >= (int32_t)m_size)
		gridX = (int32_t)m_size - 1;

	if (gridZ < 0)
		gridZ = 0;
	else if (gridZ >= (int32_t)m_size)
		gridZ = (int32_t)m_size - 1;

	int32_t offset = gridX + gridZ * m_size;
	return (m_cuts[offset / 8] & (1 << (offset & 7))) != 0;
}

bool Heightfield::getWorldCut(float worldX, float worldZ) const
{
	int32_t gridX, gridZ;
	worldToGrid(worldX, worldZ, gridX, gridZ);
	return getGridCut(gridX, gridZ);
}

uint8_t Heightfield::getGridAttribute(int32_t gridX, int32_t gridZ) const
{
	if (gridX < 0)
		gridX = 0;
	else if (gridX >= (int32_t)m_size)
		gridX = (int32_t)m_size - 1;

	if (gridZ < 0)
		gridZ = 0;
	else if (gridZ >= (int32_t)m_size)
		gridZ = (int32_t)m_size - 1;

	int32_t offset = gridX + gridZ * m_size;
	return m_attributes[offset];
}

uint8_t Heightfield::getWorldAttribute(float worldX, float worldZ) const
{
	int32_t gridX, gridZ;
	worldToGrid(worldX, worldZ, gridX, gridZ);
	return getGridAttribute(gridX, gridZ);
}

void Heightfield::gridToWorld(int32_t gridX, int32_t gridZ, float& outWorldX, float& outWorldZ) const
{
	gridToWorld(float(gridX), float(gridZ), outWorldX, outWorldZ);
}

void Heightfield::gridToWorld(float gridX, float gridZ, float& outWorldX, float& outWorldZ) const
{
	outWorldX = m_worldExtentFloats[0] * (gridX / m_size - 0.5f) + 0.5f;
	outWorldZ = m_worldExtentFloats[2] * (gridZ / m_size - 0.5f) + 0.5f;
}

Vector4 Heightfield::gridToWorld(float gridX, float gridZ) const
{
	float worldX, worldZ;
	gridToWorld(gridX, gridZ, worldX, worldZ);
	return Vector4(
		worldX,
		getWorldHeight(worldX, worldZ),
		worldZ,
		1.0f
	);
}

void Heightfield::worldToGrid(float worldX, float worldZ, int32_t& outGridX, int32_t& outGridZ) const
{
	float gridX, gridZ;
	worldToGrid(worldX, worldZ, gridX, gridZ);
	outGridX = (int32_t)gridX;
	outGridZ = (int32_t)gridZ;
}

void Heightfield::worldToGrid(float worldX, float worldZ, float& outGridX, float& outGridZ) const
{
	outGridX = m_size * (worldX - 0.5f + m_worldExtentFloats[0] * 0.5f) / m_worldExtentFloats[0];
	outGridZ = m_size * (worldZ - 0.5f + m_worldExtentFloats[2] * 0.5f) / m_worldExtentFloats[2];
}

float Heightfield::unitToWorld(float unitY) const
{
	float wexy = m_worldExtentFloats[1];
	return -wexy * 0.5f + unitY * wexy;
}

float Heightfield::worldToUnit(float worldY) const
{
	float wexy = m_worldExtentFloats[1];
	return (worldY + wexy * 0.5f) / wexy;
}

Vector4 Heightfield::normalAt(float gridX, float gridZ) const
{
	const float c_distance = 0.5f;
	const float directions[][2] =
	{
		{ -c_distance, -c_distance },
		{        0.0f, -c_distance },
		{  c_distance, -c_distance },
		{  c_distance,        0.0f },
		{  c_distance,  c_distance },
		{        0.0f,        0.0f },
		{ -c_distance,  c_distance },
		{ -c_distance,        0.0f }
	};

	float h0 = getGridHeightBilinear(gridX, gridZ);

	float h[sizeof_array(directions)];
	for (uint32_t i = 0; i < sizeof_array(directions); ++i)
		h[i] = getGridHeightBilinear(gridX + directions[i][0], gridZ + directions[i][1]);

	const Vector4& worldExtent = getWorldExtent();
	float sx = worldExtent.x() / getSize();
	float sy = worldExtent.y();
	float sz = worldExtent.z() / getSize();

	Vector4 N = Vector4::zero();

	for (uint32_t i = 0; i < sizeof_array(directions); ++i)
	{
		uint32_t j = (i + 1) % sizeof_array(directions);

		float dx1 = directions[i][0] * sx * c_distance;
		float dy1 = (h[i] - h0) * sy;
		float dz1 = directions[i][1] * sz * c_distance;

		float dx2 = directions[j][0] * sx * c_distance;
		float dy2 = (h[j] - h0) * sy;
		float dz2 = directions[j][1] * sz * c_distance;

		Vector4 n = cross(
			Vector4(dx2, dy2, dz2),
			Vector4(dx1, dy1, dz1)
		);

		N += n;
	}

	return N.normalized();
}

bool Heightfield::queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const
{
	const int32_t c_cellSize = 64;
	const int32_t c_skip = 4;

	Scalar k;
	Scalar kIn, kOut;

	Aabb3 boundingBox(-m_worldExtent * Scalar(0.5f), m_worldExtent * Scalar(0.5f));
	if (!boundingBox.intersectRay(worldRayOrigin, worldRayDirection, kIn, kOut))
		return false;

	bool foundIntersection = false;

	outDistance = Scalar(std::numeric_limits< float >::max());

	for (int32_t cz = 0; cz < m_size; cz += c_cellSize)
	{
		for (int32_t cx = 0; cx < m_size; cx += c_cellSize)
		{
			float cx1w, cz1w;
			float cx2w, cz2w;

			gridToWorld(float(cx), float(cz), cx1w, cz1w);
			gridToWorld(float(cx + c_cellSize), float(cz + c_cellSize), cx2w, cz2w);

			float cyw[] =
			{
				unitToWorld(getGridHeightNearest(cx, cz)),
				unitToWorld(getGridHeightNearest(cx + c_cellSize, cz)),
				unitToWorld(getGridHeightNearest(cx, cz + c_cellSize)),
				unitToWorld(getGridHeightNearest(cx + c_cellSize, cz + c_cellSize))
			};

			float cy1w = *std::min_element(cyw, cyw + sizeof_array(cyw));
			float cy2w = *std::max_element(cyw, cyw + sizeof_array(cyw));

			Aabb3 bb;
			bb.mn = Vector4(cx1w, cy1w, cz1w, 1.0f);
			bb.mx = Vector4(cx2w, cy2w, cz2w, 1.0f);

			if (!bb.intersectRay(worldRayOrigin, worldRayDirection, kIn, kOut))
				continue;

			for (int32_t iz = cz; iz <= cz + c_cellSize; iz += c_skip)
			{
				for (int32_t ix = cx; ix <= cx + c_cellSize; ix += c_skip)
				{
					float x1w, z1w;
					float x2w, z2w;

					gridToWorld(float(ix), float(iz), x1w, z1w);
					gridToWorld(float(ix + c_skip), float(iz + c_skip), x2w, z2w);

					float yw[] =
					{
						unitToWorld(getGridHeightNearest(ix, iz)),
						unitToWorld(getGridHeightNearest(ix + c_skip, iz)),
						unitToWorld(getGridHeightNearest(ix, iz + c_skip)),
						unitToWorld(getGridHeightNearest(ix + c_skip, iz + c_skip))
					};

					Vector4 vw[] =
					{
						Vector4(x1w, yw[0], z1w, 1.0f),
						Vector4(x2w, yw[1], z1w, 1.0f),
						Vector4(x1w, yw[2], z2w, 1.0f),
						Vector4(x2w, yw[3], z2w, 1.0f)
					};

					if (Winding3(vw[0], vw[1], vw[2]).rayIntersection(worldRayOrigin, worldRayDirection, k))
					{
						if (k < outDistance)
						{
							outDistance = k;
							foundIntersection = true;
						}
					}

					if (Winding3(vw[1], vw[3], vw[2]).rayIntersection(worldRayOrigin, worldRayDirection, k))
					{
						if (k < outDistance)
						{
							outDistance = k;
							foundIntersection = true;
						}
					}
				}
			}
		}
	}

	return foundIntersection;
}

	}
}
