/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Const.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/Editor/ConvertHeightfield.h"
#include "Model/Model.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace traktor::hf
{
	namespace
	{

const float c_forceSplit = std::numeric_limits< float >::max();

int64_t floorDiv(int64_t a, int64_t b)
{
	int64_t q = a / b;
	if ((a % b) != 0 && ((a < 0) != (b < 0)))
		--q;
	return q;
}

int64_t ceilDiv(int64_t a, int64_t b)
{
	return -floorDiv(-a, b);
}

/*! Maximum vertical distance, in unit heights, from the heightfield samples covered by a triangle to the triangle.
 *
 * Corners are in grid coordinates and may lie outside of the heightfield, in which
 * case the corner height is clamped and samples outside are ignored.
 */
float calculateTriangleError(const Heightfield* heightfield, int32_t ax, int32_t az, int32_t bx, int32_t bz, int32_t cx, int32_t cz)
{
	const int32_t size = heightfield->getSize();
	const height_t* heights = heightfield->getHeights();

	const float ha = heightfield->getGridHeightNearest(ax, az);
	const float hb = heightfield->getGridHeightNearest(bx, bz);
	const float hc = heightfield->getGridHeightNearest(cx, cz);

	const int64_t e1x = bx - ax, e1z = bz - az;
	const int64_t e2x = cx - ax, e2z = cz - az;
	const int64_t det = e1x * e2z - e1z * e2x;
	if (det == 0)
		return 0.0f;

	// Plane through corners, h(x, z) = ha + dhdx * (x - ax) + dhdz * (z - az).
	const float dhdx = ((hb - ha) * e2z - (hc - ha) * e1z) / det;
	const float dhdz = ((hc - ha) * e1x - (hb - ha) * e2x) / det;

	const int32_t edges[][4] = {
		{ ax, az, bx - ax, bz - az },
		{ bx, bz, cx - bx, cz - bz },
		{ cx, cz, ax - cx, az - cz }
	};
	const int64_t sign = (det > 0) ? 1 : -1;

	const int32_t minX = std::max(std::min({ ax, bx, cx }), 0);
	const int32_t maxX = std::min(std::max({ ax, bx, cx }), size - 1);
	const int32_t minZ = std::max(std::min({ az, bz, cz }), 0);
	const int32_t maxZ = std::min(std::max({ az, bz, cz }), size - 1);

	float maxError = 0.0f;
	for (int32_t z = minZ; z <= maxZ; ++z)
	{
		// Intersect row with the inside half-plane of each edge.
		int64_t x0 = minX;
		int64_t x1 = maxX;
		for (const auto& edge : edges)
		{
			const int64_t k = int64_t(edge[2]) * (z - edge[1]) + int64_t(edge[3]) * edge[0];
			const int64_t d = edge[3];
			if (d == 0)
			{
				if (sign * k < 0)
					x1 = x0 - 1;
			}
			else if (sign * d > 0)
				x1 = std::min(x1, floorDiv(k, d));
			else
				x0 = std::max(x0, ceilDiv(k, d));
		}

		const float rowHeight = ha + dhdz * float(z - az);
		const height_t* row = heights + z * size;
		for (int64_t x = x0; x <= x1; ++x)
		{
			const float h = rowHeight + dhdx * float(x - ax);
			maxError = std::max(maxError, std::abs(row[x] / 65535.0f - h));
		}
	}
	return maxError;
}

/*! Lattice of heightfield samples, every step sample, used for adaptive tessellation.
 *
 * The lattice is tessellated as a right-triangulated irregular network; each triangle
 * is recursively split at the midpoint of its hypotenuse. The error of a triangle is
 * stored in its hypotenuse midpoint, which is shared with the neighbour triangle, and
 * includes the error of all descendants. Thus splitting a triangle also split its
 * neighbour and the resulting mesh is free from T-junctions.
 */
struct Lattice
{
	const Heightfield* heightfield;
	int32_t step;
	int32_t dim;	//!< Number of lattice cells along each axis, power of two.
	AlignedVector< float > errors;
	AlignedVector< uint8_t > present;

	int32_t index(int32_t x, int32_t z) const { return x + z * (dim + 1); }

	float triangleError(int32_t ax, int32_t az, int32_t bx, int32_t bz, int32_t cx, int32_t cz) const
	{
		return calculateTriangleError(heightfield, ax * step, az * step, bx * step, bz * step, cx * step, cz * step);
	}
};

void calculateLatticeErrors(Lattice& lattice)
{
	const int32_t dim = lattice.dim;
	const int32_t count = dim + 1;
	AlignedVector< float >& errors = lattice.errors;

	// Force full resolution where terrain is cut or at the lattice boundary, as triangles
	// are only emitted when all corners are present.
	for (int32_t z = 0; z < count; ++z)
	{
		for (int32_t x = 0; x < count; ++x)
		{
			bool anyPresent = false;
			bool anyAbsent = false;
			for (int32_t iz = std::max(z - 1, 0); iz <= std::min(z + 1, dim); ++iz)
			{
				for (int32_t ix = std::max(x - 1, 0); ix <= std::min(x + 1, dim); ++ix)
				{
					if (lattice.present[lattice.index(ix, iz)])
						anyPresent = true;
					else
						anyAbsent = true;
				}
			}
			if (anyPresent && anyAbsent)
				errors[lattice.index(x, z)] = c_forceSplit;
		}
	}

	// Calculate errors bottom-up; each square size has two levels of triangles.
	for (int32_t s = 2; s <= dim; s <<= 1)
	{
		const int32_t h = s >> 1;

		// Triangles with hypotenuse along the sides of each square, right angle at square center.
		for (int32_t z = 0; z < dim; z += s)
		{
			for (int32_t x = 0; x < dim; x += s)
			{
				const int32_t mx = x + h;
				const int32_t mz = z + h;
				const int32_t sides[][4] = {
					{ x, z, x + s, z },
					{ x + s, z, x + s, z + s },
					{ x + s, z + s, x, z + s },
					{ x, z + s, x, z }
				};
				for (const auto& side : sides)
				{
					float& error = errors[lattice.index((side[0] + side[2]) >> 1, (side[1] + side[3]) >> 1)];
					error = std::max(error, lattice.triangleError(side[0], side[1], side[2], side[3], mx, mz));

					// Children are halves of the sub-squares adjacent to the side; unless they are leaves.
					if (h > 1)
					{
						error = std::max(error, errors[lattice.index((side[0] + mx) >> 1, (side[1] + mz) >> 1)]);
						error = std::max(error, errors[lattice.index((side[2] + mx) >> 1, (side[3] + mz) >> 1)]);
					}
				}
			}
		}

		// Triangles with hypotenuse along the diagonal of each square, alternating diagonal
		// direction such that every diagonal pass through the center of the parent square.
		for (int32_t z = 0; z < dim; z += s)
		{
			for (int32_t x = 0; x < dim; x += s)
			{
				const int32_t mx = x + h;
				const int32_t mz = z + h;

				float& error = errors[lattice.index(mx, mz)];
				if ((((x + z) / s) & 1) == 0)
				{
					error = std::max(error, lattice.triangleError(x, z, x + s, z + s, x + s, z));
					error = std::max(error, lattice.triangleError(x + s, z + s, x, z, x, z + s));
				}
				else
				{
					error = std::max(error, lattice.triangleError(x + s, z, x, z + s, x, z));
					error = std::max(error, lattice.triangleError(x, z + s, x + s, z, x + s, z + s));
				}

				// Children are the triangles along the square sides.
				error = std::max(error, errors[lattice.index(mx, z)]);
				error = std::max(error, errors[lattice.index(x + s, mz)]);
				error = std::max(error, errors[lattice.index(mx, z + s)]);
				error = std::max(error, errors[lattice.index(x, mz)]);
			}
		}
	}
}

//! Recursively collect triangles, c is the right angle corner.
void extractLatticeTriangles(const Lattice& lattice, float maxError, int32_t ax, int32_t az, int32_t bx, int32_t bz, int32_t cx, int32_t cz, AlignedVector< int32_t >& outIndices)
{
	const int32_t mx = (ax + bx) >> 1;
	const int32_t mz = (az + bz) >> 1;

	// Leaf triangles have a hypotenuse midpoint which isn't on the lattice.
	if (std::abs(ax - cx) + std::abs(az - cz) > 1 && lattice.errors[lattice.index(mx, mz)] > maxError)
	{
		extractLatticeTriangles(lattice, maxError, cx, cz, ax, az, mx, mz, outIndices);
		extractLatticeTriangles(lattice, maxError, bx, bz, cx, cz, mx, mz, outIndices);
		return;
	}

	const int32_t ia = lattice.index(ax, az);
	const int32_t ib = lattice.index(bx, bz);
	const int32_t ic = lattice.index(cx, cz);
	if (!lattice.present[ia] || !lattice.present[ib] || !lattice.present[ic])
		return;

	// Same winding as the regular grid.
	outIndices.push_back(ia);
	if ((bx - ax) * (cz - az) - (bz - az) * (cx - ax) > 0)
	{
		outIndices.push_back(ib);
		outIndices.push_back(ic);
	}
	else
	{
		outIndices.push_back(ic);
		outIndices.push_back(ib);
	}
}

void tessellateAdaptive(const Heightfield* heightfield, int32_t step, float maxError, model::Model* model, uint32_t baseChannel, uint32_t lightmapChannel)
{
	const int32_t size = heightfield->getSize();
	if (size < 2)
		return;

	step = std::max(step, 1);

	// Smallest power-of-two lattice which cover the whole heightfield.
	int32_t cells = (size - 1 + step - 1) / step;

	Lattice lattice;
	lattice.heightfield = heightfield;
	lattice.step = step;
	lattice.dim = 1;
	while (lattice.dim < cells)
		lattice.dim <<= 1;

	// Include one sample past the last, clamped the same way as the terrain height map is sampled,
	// if that align the edge with the lattice; otherwise the edge must be tessellated at full resolution.
	cells = std::min(std::max(cells, size / step), lattice.dim);

	const int32_t count = lattice.dim + 1;
	lattice.errors.resize(count * count, 0.0f);
	lattice.present.resize(count * count, 0);

	for (int32_t z = 0; z <= cells; ++z)
	{
		for (int32_t x = 0; x <= cells; ++x)
			lattice.present[lattice.index(x, z)] = heightfield->getGridCut(x * step, z * step) ? 1 : 0;
	}

	calculateLatticeErrors(lattice);

	const float maxUnitError = maxError / heightfield->getWorldExtent().y();
	const int32_t dim = lattice.dim;

	AlignedVector< int32_t > indices;
	extractLatticeTriangles(lattice, maxUnitError, 0, 0, dim, dim, dim, 0, indices);
	extractLatticeTriangles(lattice, maxUnitError, dim, dim, 0, 0, 0, dim, indices);

	AlignedVector< uint32_t > vertexIds(count * count, model::c_InvalidIndex);

	model->reservePolygons(uint32_t(indices.size() / 3));

	model::Vertex vertex;
	model::Polygon polygon;
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		polygon.clearVertices();
		polygon.setMaterial(0);

		for (size_t j = 0; j < 3; ++j)
		{
			const int32_t index = indices[i + j];
			if (vertexIds[index] == model::c_InvalidIndex)
			{
				const int32_t gx = (index % count) * step;
				const int32_t gz = (index / count) * step;

				float wx, wz;
				heightfield->gridToWorld(gx, gz, wx, wz);

				const uint32_t positionId = model->addPosition(Vector4(
					wx,
					heightfield->unitToWorld(heightfield->getGridHeightNearest(gx, gz)),
					wz,
					1.0f
				));

				const Vector4 normal = heightfield->normalAt(float(std::min(gx, size - 1)), float(std::min(gz, size - 1)));
				const uint32_t normalId = model->addNormal(normal.xyz0());

				const uint32_t texCoordId = model->addTexCoord(Vector2(
					(gx + 0.5f) / size,
					(gz + 0.5f) / size
				));

				vertex.setPosition(positionId);
				vertex.setNormal(normalId);
				vertex.setTexCoord(baseChannel, texCoordId);
				vertex.setTexCoord(lightmapChannel, texCoordId);

				vertexIds[index] = model->addVertex(vertex);
			}
			polygon.addVertex(vertexIds[index]);
		}

		model->addPolygon(polygon);
	}
}

void tessellateRegular(const Heightfield* heightfield, int32_t step, model::Model* model, uint32_t baseChannel, uint32_t lightmapChannel)
{
	int32_t size = heightfield->getSize();

	int32_t ix0, iz0;
	int32_t ix1, iz1;

	ix0 = 0;
	iz0 = 0;
	ix1 = size;
	iz1 = size;

	size = max(ix1 - ix0, iz1 - iz0);

	const int32_t outputSize = max(size / step, 1);

	// Convert vertices.
	model->reservePositions(outputSize * outputSize);
	model->reserveNormals(outputSize * outputSize);
	model->reserveVertices(outputSize * outputSize);

	model::Vertex vertex;
	for (int32_t iz = 0; iz < outputSize; ++iz)
	{
		for (int32_t ix = 0; ix < outputSize; ++ix)
		{
			float wx, wz;
			heightfield->gridToWorld(ix0 + ix * step, iz0 + iz * step, wx, wz);

			const uint32_t positionId = model->addPosition(Vector4(
				wx,
				heightfield->getWorldHeight(wx, wz),
				wz,
				1.0f
			));

			const Vector4 normal = heightfield->normalAt(ix0 + ix * step, iz0 + iz * step);
			const uint32_t normalId = model->addNormal(normal.xyz0());

			const uint32_t texCoordId = model->addTexCoord(Vector2(
				float(ix) / (outputSize - 1),
				float(iz) / (outputSize - 1)
			));

			vertex.setPosition(positionId);
			vertex.setNormal(normalId);
			vertex.setTexCoord(baseChannel, texCoordId);
			vertex.setTexCoord(lightmapChannel, texCoordId);

			model->addVertex(vertex);
		}
	}

	// Convert polygons.
	model::Polygon polygon;
	for (int32_t iz = 0; iz < outputSize - 1; ++iz)
	{
		const int32_t offset = iz * outputSize;
		for (int32_t ix = 0; ix < outputSize - 1; ++ix)
		{
			const int32_t gx = ix0 + ix * step;
			const int32_t gz = iz0 + iz * step;

			if (!heightfield->getGridCut(gx, gz))
				continue;
			if (!heightfield->getGridCut(gx + step, gz))
				continue;
			if (!heightfield->getGridCut(gx + step, gz + step))
				continue;
			if (!heightfield->getGridCut(gx, gz + step))
				continue;

			const float heights[] =
			{
				heightfield->getGridHeightNearest(gx, gz),
				heightfield->getGridHeightNearest(gx + step, gz),
				heightfield->getGridHeightNearest(gx + step, gz + step),
				heightfield->getGridHeightNearest(gx, gz + step)
			};

			const float ch = (heights[0] + heights[1] + heights[2] + heights[3]) / 4.0f;

			const float dh[] =
			{
				std::abs(ch - heights[0]),
				std::abs(ch - heights[1]),
				std::abs(ch - heights[2]),
				std::abs(ch - heights[3])
			};

			const auto it = std::max_element(&dh[0], &dh[4]);
			const int32_t mdhi = (int32_t)std::distance(&dh[0], it);

			const int32_t indices[] =
			{
				offset + ix,
				offset + ix + 1,
				offset + ix + 1 + outputSize,
				offset + ix + outputSize
			};

			if (mdhi == 0 || mdhi == 2)
			{
				polygon.clearVertices();
				polygon.setMaterial(0);
				polygon.addVertex(indices[0]);
				polygon.addVertex(indices[1]);
				polygon.addVertex(indices[3]);
				model->addPolygon(polygon);

				polygon.clearVertices();
				polygon.setMaterial(0);
				polygon.addVertex(indices[1]);
				polygon.addVertex(indices[2]);
				polygon.addVertex(indices[3]);
				model->addPolygon(polygon);
			}
			else
			{
				polygon.clearVertices();
				polygon.setMaterial(0);
				polygon.addVertex(indices[0]);
				polygon.addVertex(indices[1]);
				polygon.addVertex(indices[2]);
				model->addPolygon(polygon);

				polygon.clearVertices();
				polygon.setMaterial(0);
				polygon.addVertex(indices[0]);
				polygon.addVertex(indices[2]);
				polygon.addVertex(indices[3]);
				model->addPolygon(polygon);
			}
		}
	}
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.ConvertHeightfield", ConvertHeightfield, Object)

Ref< model::Model > ConvertHeightfield::convert(const Heightfield* heightfield, int32_t step, float maxError) const
{
	Ref< model::Model > model = new model::Model();

	// Add texcoord channels.
	const uint32_t baseChannel = model->addUniqueTexCoordChannel(L"Base");
	const uint32_t lightmapChannel = model->addUniqueTexCoordChannel(L"Lightmap");

	// Add single material for entire heightfield.
	model::Material material;
	material.setName(L"Heightfield");
	model->addMaterial(material);

	if (maxError > 0.0f)
		tessellateAdaptive(heightfield, step, maxError, model, baseChannel, lightmapChannel);
	else
		tessellateRegular(heightfield, step, model, baseChannel, lightmapChannel);

	return model;
}

}
