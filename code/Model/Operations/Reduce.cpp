/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Containers/StaticSet.h"
#include "Core/Math/Const.h"
#include "Core/Math/Plane.h"
#include "Model/Model.h"
#include "Model/ModelAdjacency.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/Reduce.h"
#include "Model/Operations/Triangulate.h"

// Loosely based on following paper
// http://www.jofcis.com/publishedpapers/2013_9_11_4271_4279.pdf

namespace traktor::model
{
	namespace
	{

Scalar tetrahedronVolume(const Vector4& A, const Vector4& B, const Vector4& C, const Vector4& u)
{
	const Scalar area = cross(A - B, C - B).length() / 2.0_simd;
	return abs((1.0_simd / 3.0_simd) * area * Plane(A, B, C).distance(u));
}

bool isTriangleDegenerate(const Model& model, uint32_t triangleId)
{
	const Polygon& polygon = model.getPolygon(triangleId);
	if (polygon.getVertexCount() != 3)
		return true;

	auto& vertices = polygon.getVertices();
	for (size_t j = 0; j < vertices.size(); ++j)
	{
		const uint32_t p0 = model.getVertex(vertices[j]).getPosition();
		const uint32_t p1 = model.getVertex(vertices[(j + 1) % vertices.size()]).getPosition();
		if (p0 == p1)
			return true;
	}
		
	return false;
}

Vector4 triangleNormal(const Model& model, uint32_t triangleId)
{
	const Polygon& polygon = model.getPolygon(triangleId);
	if (polygon.getVertexCount() < 3)
		return Vector4::zero();

	const auto& vertices = polygon.getVertices();
	const Vertex* v[] =
	{
		&model.getVertex(vertices[0]),
		&model.getVertex(vertices[1]),
		&model.getVertex(vertices[2])
	};

	Vector4 p[] =
	{
		model.getPosition(v[0]->getPosition()),
		model.getPosition(v[1]->getPosition()),
		model.getPosition(v[2]->getPosition())
	};

	Vector4 ep[] = { p[2] - p[0], p[1] - p[0] };
	T_ASSERT(ep[0].length() > FUZZY_EPSILON);
	T_ASSERT(ep[1].length() > FUZZY_EPSILON);

	return cross(ep[0], ep[1]).normalized();
}

Vector4 triangleMidPoint(const Model& model, uint32_t triangleId)
{
	const Polygon& polygon = model.getPolygon(triangleId);
	if (polygon.getVertexCount() < 3)
		return Vector4::zero();

	Vector4 midPoint = Vector4::zero();
	for (uint32_t j = 0; j < 3; ++j)
		midPoint += model.getVertexPosition(polygon.getVertex(j));

	return (midPoint / Scalar(3.0f)).xyz1();
}

Vector2 triangleMidTexCoord(const Model& model, uint32_t triangleId)
{
	const Polygon& polygon = model.getPolygon(triangleId);
	if (polygon.getVertexCount() < 3)
		return Vector2::zero();

	Vector2 midPoint = Vector2::zero();
	for (uint32_t j = 0; j < 3; ++j)
		midPoint += model.getTexCoord(model.getVertex(polygon.getVertex(j)).getTexCoord(0));

	return midPoint / 3.0f;
}

Vector4 triangleTipPoint(const Model& model, const ModelAdjacency& adjacency, uint32_t triangleId)
{
	const Polygon& polygon = model.getPolygon(triangleId);
	if (polygon.getVertexCount() < 3)
		return Vector4::zero();

	const Vector4 normal = triangleNormal(model, triangleId);
	const Vector4 midPoint = triangleMidPoint(model, triangleId);

	ModelAdjacency::share_vector_t sharedEdges;
	Vector4 tipPoint = Vector4::zero();
	int32_t count = 0;

	for (uint32_t j = 0; j < 3; ++j)
	{
		sharedEdges = adjacency.getSharedEdges(triangleId, j);
		if (sharedEdges.size() > 0)
		{
			const uint32_t sharedTriangleId = adjacency.getPolygon(sharedEdges[0]);

			const Vector4 sharedMidPoint = triangleMidPoint(model, sharedTriangleId);
			const Vector4 diff = midPoint - sharedMidPoint;

			const Vector4 position0 = model.getVertexPosition(polygon.getVertex(j));
			const Vector4 position1 = model.getVertexPosition(polygon.getVertex((j + 1) % 3));

			const Vector4 edge = position1 - position0;

			Vector4 pivot;
			Scalar k;

			if (!Plane(cross(normal, diff).normalized(), midPoint).intersectRay(position0, edge.normalized(), k, pivot))
				continue;

			const Vector4 V = (pivot - sharedMidPoint).normalized();

			const Scalar kd = dot3((midPoint - pivot).normalized(), V);
			if (kd <= FUZZY_EPSILON)
				continue;

			tipPoint += pivot + V * (midPoint - pivot).length() / kd;
			count++;
		}
	}

	if (count > 0)
		tipPoint = (tipPoint / Scalar((float)count)).xyz1();
	else
		tipPoint = midPoint;

	return tipPoint;
}

void triangleEdgeNeighbors(const Model& model, const ModelAdjacency& adjacency, uint32_t triangleId, AlignedVector< uint32_t >& outNeighborTriangleIds)
{
	const Polygon& polygon = model.getPolygon(triangleId);
	if (polygon.getVertexCount() < 3)
		return;

	// Get edge neighbors.
	ModelAdjacency::share_vector_t sharedEdges;
	for (uint32_t j = 0; j < 3; ++j)
	{
		sharedEdges = adjacency.getSharedEdges(triangleId, j);
		if (sharedEdges.size() > 0)
		{
			const uint32_t sharedTriangleId = adjacency.getPolygon(sharedEdges[0]);
			outNeighborTriangleIds.push_back(sharedTriangleId);
		}
	}
}

void triangleSingleVertexNeighbors(const Model& model, uint32_t triangleId, AlignedVector< std::pair< uint32_t, uint32_t > >& outNeighborTriangleIds)
{
	const Polygon& polygon = model.getPolygon(triangleId);
	if (polygon.getVertexCount() < 3)
		return;

	// Get single vertex neighbors.
	uint32_t positionIds[3];
	for (uint32_t i = 0; i < 3; ++i)
		positionIds[i] = model.getVertex(polygon.getVertex(i)).getPosition();

	for (uint32_t i = 0; i < model.getPolygonCount(); ++i)
	{
		if (i == triangleId)
			continue;

		const auto& vertices = model.getPolygon(i).getVertices();
		if (vertices.size() != 3)
			continue;

		uint32_t numSharingVertices = 0;
		uint32_t sharingVertexId = 0;

		for (uint32_t j = 0; j < 3; ++j)
		{
			const uint32_t sharedPositionId = model.getVertex(vertices[j]).getPosition();
			if (
				sharedPositionId == positionIds[0] ||
				sharedPositionId == positionIds[1] ||
				sharedPositionId == positionIds[2]
			)
			{
				numSharingVertices++;
				sharingVertexId = vertices[j];
			}
		}

		if (numSharingVertices == 1)
			outNeighborTriangleIds.push_back(std::make_pair(i, sharingVertexId));
	}
}

float triangleVolumeError(const Model& model, const ModelAdjacency& adjacency, uint32_t triangleId)
{
	const Polygon& polygon = model.getPolygon(triangleId);
	if (polygon.getVertexCount() < 3)
		return 0.0f;

	const Vector4 tipPoint = triangleTipPoint(model, adjacency, triangleId);

	uint32_t positionIds[3];
	for (uint32_t i = 0; i < 3; ++i)
		positionIds[i] = model.getVertex(polygon.getVertex(i)).getPosition();

	// Calculate volume error of triangle.
	float error = tetrahedronVolume(
		model.getPosition(positionIds[0]),
		model.getPosition(positionIds[1]),
		model.getPosition(positionIds[2]),
		tipPoint
	);

	// Accumulate volume errors of edge sharing triangles.
	ModelAdjacency::share_vector_t sharedEdges;
	for (uint32_t j = 0; j < 3; ++j)
	{
		sharedEdges = adjacency.getSharedEdges(triangleId, j);
		if (sharedEdges.size() > 0)
		{
			const uint32_t sharedTriangleId = adjacency.getPolygon(sharedEdges[0]);
			const Polygon& sharedTriangle = model.getPolygon(sharedTriangleId);
			error += tetrahedronVolume(
				model.getVertexPosition(sharedTriangle.getVertex(0)),
				model.getVertexPosition(sharedTriangle.getVertex(1)),
				model.getVertexPosition(sharedTriangle.getVertex(2)),
				tipPoint
			);
		}
		else
		{
			// Triangles with no neighbors shouldn't be discarded;
			// thus return max error.
			return std::numeric_limits< float >::max();
		}
	}

	// Accumulate volume errors of single vertex sharing triangles.
	AlignedVector< std::pair< uint32_t, uint32_t > > neighborTriangleIds;
	triangleSingleVertexNeighbors(model, triangleId, neighborTriangleIds);

	// Triangle with less than 3 single vertex sharing triangles shouldn't be discarded; thus return max error.
	if (neighborTriangleIds.size() < 3)
		return std::numeric_limits< float >::max();

	for (const auto& neighborTriangleId : neighborTriangleIds)
	{
		const Polygon& sharedTriangle = model.getPolygon(neighborTriangleId.first);
		error += tetrahedronVolume(
			model.getVertexPosition(sharedTriangle.getVertex(0)),
			model.getVertexPosition(sharedTriangle.getVertex(1)),
			model.getVertexPosition(sharedTriangle.getVertex(2)),
			tipPoint
		);
	}

	return error;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.Reduce", Reduce, IModelOperation)

Reduce::Reduce(float target)
:	m_target(target)
{
}

bool Reduce::apply(Model& model) const
{
	// Model must be triangulated.
	model.apply(Triangulate());

	// Prepare initial adjacency.
	Ref< ModelAdjacency > adjacency = new ModelAdjacency(&model, ModelAdjacency::Mode::ByPosition);

	// Calculate initial set of errors.
	AlignedVector< float > errors;
	for (uint32_t i = 0; i < model.getPolygonCount(); ++i)
	{
		const float error = triangleVolumeError(model, *adjacency, i);
		errors.push_back(error);
	}

	StaticVector< uint32_t, 4 > errorTrianglePositionIds;
	StaticVector< uint32_t, 64 > modifiedPolygons;
	StaticSet< uint32_t, 64 > modifiedPositions;

	// Iterate and discard triangles until target is meet.
	const uint32_t targetPolygonCount = (int32_t)(model.getPolygonCount() * m_target + 0.5f);
	while (model.getPolygonCount() > targetPolygonCount)
	{
		// Find triangle with smallest error to collapse.
		// One minus max so error function can force some triangles to not even
		// being taken into account.
		uint32_t minErrorTriangleId = c_InvalidIndex;
		float minError = std::numeric_limits< float >::max() - 1.0f;
		for (uint32_t i = 0; i < model.getPolygonCount(); ++i)
		{
			const float error = errors[i];
			if (error < minError)
			{
				minErrorTriangleId = i;
				minError = error;
			}
		}
		if (minErrorTriangleId == c_InvalidIndex)
			break;

		errorTrianglePositionIds.resize(0);
		modifiedPositions.reset();

		// Get all position ids from collapsing triangle.
		for (uint32_t vertex : model.getPolygon(minErrorTriangleId).getVertices())
			errorTrianglePositionIds.push_back(model.getVertex(vertex).getPosition());

		// Calculate new join position.
		const Vector4 tipPoint = triangleTipPoint(model, *adjacency, minErrorTriangleId);
		const Vector4 midPoint = triangleMidPoint(model, minErrorTriangleId);
		const Vector2 midTexCoord = triangleMidTexCoord(model, minErrorTriangleId);
		const Vector4 joinPoint = lerp(tipPoint, midPoint, 0.6_simd).xyz1();
		const Vector2 joinTexCoord = midTexCoord;
		const uint32_t joinPointId = model.addUniquePosition(joinPoint);
		const uint32_t joinTexCoordId = model.addUniqueTexCoord(joinTexCoord);

		// Replace all vertices which reference any of the collapsing triangle's positions.
		auto& polygons = model.getPolygons();
		for (uint32_t i = 0; i < (uint32_t)polygons.size(); )
		{
			Polygon& polygon = polygons[i];
			bool polygonModified = false;

			for (uint32_t j = 0; j < polygon.getVertexCount(); ++j)
			{
				Vertex vrtx = model.getVertex(polygon.getVertex(j));
				const auto it = std::find(errorTrianglePositionIds.begin(), errorTrianglePositionIds.end(), vrtx.getPosition());
				if (it != errorTrianglePositionIds.end())
				{
					vrtx.setPosition(joinPointId);
					vrtx.setTexCoord(0, joinTexCoordId);
					const uint32_t replaceVrtxId = model.addUniqueVertex(vrtx);
					polygon.setVertex(j, replaceVrtxId);
					polygonModified = true;
				}
			}

			if (polygonModified && isTriangleDegenerate(model, i))
			{
				polygons.erase(polygons.begin() + i);
				errors.erase(errors.begin() + i);
				adjacency->remove(i, true);
			}
			else
			{
				if (polygonModified)
				{
					for (uint32_t vertexId : polygon.getVertices())
					{
						const Vertex& vrtx = model.getVertex(vertexId);
						modifiedPositions.insert(vrtx.getPosition());
					}
				}
				++i;
			}
		}

		// Get array of all modified polygons, directly or indirectly.
		modifiedPolygons.resize(0);
		for (uint32_t i = 0; i < model.getPolygonCount(); ++i)
		{
			const Polygon& polygon = model.getPolygon(i);
			const Polygon::vertices_t& vertices = polygon.getVertices();
			const auto it = std::find_if(vertices.begin(), vertices.end(), [&](uint32_t vertexId) {
				const uint32_t positionId = model.getVertex(vertexId).getPosition();
				return modifiedPositions.find(positionId) != modifiedPositions.end();
			});
			if (it != vertices.end())
				modifiedPolygons.push_back(i);
		}

		// Update adjacency.
		for (uint32_t modifiedPolygon : modifiedPolygons)
			adjacency->update(modifiedPolygon);

		// Update errors on polygons which has been modified.
		for (uint32_t modifiedPolygon : modifiedPolygons)
			errors[modifiedPolygon] = triangleVolumeError(model, *adjacency, modifiedPolygon);
	}

	// Remove unused vertices etc which will be a left over from reducing.
	model.apply(CleanDuplicates(FUZZY_EPSILON));
	return true;
}

}
