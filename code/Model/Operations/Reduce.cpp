#include <limits>
#include "Core/Math/Const.h"
#include "Core/Math/Plane.h"
#include "Model/Model.h"
#include "Model/ModelAdjacency.h"
#include "Model/Operations/Reduce.h"
#include "Model/Operations/Triangulate.h"

// Loosely based on following paper
// http://www.jofcis.com/publishedpapers/2013_9_11_4271_4279.pdf

namespace traktor
{
	namespace model
	{
		namespace
		{

Scalar tetrahedronVolume(const Vector4& A, const Vector4& B, const Vector4& C, const Vector4& u)
{
	Scalar area = cross(A - B, C - B).length() / Scalar(2.0f);
	return abs(Scalar(1.0f / 3.0f) * area * Plane(A, B, C).distance(u));
}

Vector4 triangleNormal(const Model& model, uint32_t triangleId)
{
	const Polygon& polygon = model.getPolygon(triangleId);
	if (polygon.getVertexCount() < 3)
		return Vector4::zero();

	const std::vector< uint32_t >& vertices = polygon.getVertices();
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
	T_ASSERT (ep[0].length() > FUZZY_EPSILON);
	T_ASSERT (ep[1].length() > FUZZY_EPSILON);

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

	Vector4 normal = triangleNormal(model, triangleId);
	Vector4 midPoint = triangleMidPoint(model, triangleId);

	std::vector< uint32_t > sharedEdges;
	Vector4 tipPoint = Vector4::zero();
	int32_t count = 0;

	for (uint32_t j = 0; j < 3; ++j)
	{
		adjacency.getSharedEdges(triangleId, j, sharedEdges);
		if (sharedEdges.size() > 0)
		{
			uint32_t sharedTriangleId = adjacency.getPolygon(sharedEdges[0]);

			Vector4 sharedMidPoint = triangleMidPoint(model, sharedTriangleId);
			Vector4 diff = midPoint - sharedMidPoint;

			Vector4 position0 = model.getVertexPosition(polygon.getVertex(j));
			Vector4 position1 = model.getVertexPosition(polygon.getVertex((j + 1) % 3));

			Vector4 edge = position1 - position0;

			Vector4 pivot;
			Scalar k;

			if (!Plane(cross(normal, diff).normalized(), midPoint).rayIntersection(position0, edge.normalized(), k, &pivot))
				continue;

			Vector4 V = (pivot - sharedMidPoint).normalized();

			Scalar kd = dot3((midPoint - pivot).normalized(), V);
			if (kd <= FUZZY_EPSILON)
				continue;

			tipPoint += pivot + V * (midPoint - pivot).length() / kd;
			count++;
		}
	}

	if (count > 0)
		tipPoint = (tipPoint / Scalar(count)).xyz1();
	else
		tipPoint = midPoint;

	return tipPoint;
}

void triangleEdgeNeighbors(const Model& model, const ModelAdjacency& adjacency, uint32_t triangleId, std::vector< uint32_t >& outNeighborTriangleIds)
{
	const Polygon& polygon = model.getPolygon(triangleId);
	if (polygon.getVertexCount() < 3)
		return;

	// Get edge neighbors.
	std::vector< uint32_t > sharedEdges;
	for (uint32_t j = 0; j < 3; ++j)
	{
		adjacency.getSharedEdges(triangleId, j, sharedEdges);
		if (sharedEdges.size() > 0)
		{
			uint32_t sharedTriangleId = adjacency.getPolygon(sharedEdges[0]);
			outNeighborTriangleIds.push_back(sharedTriangleId);
		}
	}
}

void triangleSingleVertexNeighbors(const Model& model, uint32_t triangleId, std::vector< std::pair< uint32_t, uint32_t > >& outNeighborTriangleIds)
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

		const std::vector< uint32_t >& vertices = model.getPolygon(i).getVertices();
		if (vertices.size() != 3)
			continue;

		uint32_t numSharingVertices = 0;
		uint32_t sharingVertexId = 0;

		for (uint32_t j = 0; j < 3; ++j)
		{
			uint32_t sharedPositionId = model.getVertex(vertices[j]).getPosition();
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

	Vector4 tipPoint = triangleTipPoint(model, adjacency, triangleId);

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
	std::vector< uint32_t > sharedEdges;
	for (uint32_t j = 0; j < 3; ++j)
	{
		adjacency.getSharedEdges(triangleId, j, sharedEdges);
		if (sharedEdges.size() > 0)
		{
			uint32_t sharedTriangleId = adjacency.getPolygon(sharedEdges[0]);
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
	std::vector< std::pair< uint32_t, uint32_t > > neighborTriangleIds;
	triangleSingleVertexNeighbors(model, triangleId, neighborTriangleIds);

	// Triangle with less than 3 single vertex sharing triangles shouldn't be discarded; thus return max error.
	if (neighborTriangleIds.size() < 3)
		return std::numeric_limits< float >::max();

	for (std::vector< std::pair< uint32_t, uint32_t > >::const_iterator i = neighborTriangleIds.begin(); i != neighborTriangleIds.end(); ++i)
	{
		const Polygon& sharedTriangle = model.getPolygon(i->first);
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
	std::vector< uint32_t > sharedEdges;

	// Model must be triangulated.
	Triangulate().apply(model);

	ModelAdjacency adjacency(&model, ModelAdjacency::MdByPosition);
	std::vector< Polygon >& polygons = model.getPolygons();

	// Calculate triangle errors.
	std::vector< float > errors(polygons.size());
	for (uint32_t i = 0; i < polygons.size(); ++i)
		errors[i] = triangleVolumeError(model, adjacency, i);

	// Iterate and discard triangles until target is meet.
	int32_t targetPolygonCount = int32_t(model.getPolygonCount() * m_target + 0.5f);
	int32_t currentPolygonCount = model.getPolygonCount();
	while (currentPolygonCount > targetPolygonCount)
	{
		// Find triangle with smallest error.
		uint32_t minErrorTriangleId = c_InvalidIndex;
		float minError = std::numeric_limits< float >::max();

		for (uint32_t i = 0; i < polygons.size(); ++i)
		{
			if (errors[i] < minError)
			{
				minErrorTriangleId = i;
				minError = errors[i];
			}
		}

		// Check if unable to find an suitable triangle.
		if (minErrorTriangleId == c_InvalidIndex)
			break;

		const Polygon& minErrorTriangle = model.getPolygon(minErrorTriangleId);

		// Calculate join point and add to model.
		Vector4 tipPoint = triangleTipPoint(model, adjacency, minErrorTriangleId);
		Vector4 midPoint = triangleMidPoint(model, minErrorTriangleId);
		Vector4 joinPoint = lerp(tipPoint, midPoint, Scalar(0.6f)).xyz1();
		Vector2 joinTexCoord = triangleMidTexCoord(model, minErrorTriangleId);

		// All triangles which share a single vertex with the triangle is updated.
		std::vector< std::pair< uint32_t, uint32_t > > singleNeighborTriangleIds;
		triangleSingleVertexNeighbors(model, minErrorTriangleId, singleNeighborTriangleIds);

		// Discard first-order adjacent triangles which share edges with triangle.
		for (uint32_t i = 0; i < 3; ++i)
		{
			adjacency.getSharedEdges(minErrorTriangleId, i, sharedEdges);
			if (sharedEdges.size() != 1)
				continue;

			uint32_t sharedTriangleId = adjacency.getPolygon(sharedEdges[0]);

			polygons[sharedTriangleId].clearVertices();
			errors[sharedTriangleId] = std::numeric_limits< float >::max();
			adjacency.remove(sharedTriangleId);
			--currentPolygonCount;
		}

		// Discard triangle.
		polygons[minErrorTriangleId].clearVertices();
		errors[minErrorTriangleId] = std::numeric_limits< float >::max();
		adjacency.remove(minErrorTriangleId);
		--currentPolygonCount;

		// Update neighbor triangles.
		for (std::vector< std::pair< uint32_t, uint32_t > >::iterator i = singleNeighborTriangleIds.begin(); i != singleNeighborTriangleIds.end(); ++i)
		{
			Vertex vertex = model.getVertex(i->second);
			vertex.setPosition(model.addUniquePosition(joinPoint));
			vertex.setTexCoord(0, model.addUniqueTexCoord(joinTexCoord));
			model.setVertex(i->second, vertex);

			adjacency.update(i->first);

			errors[i->first] = triangleVolumeError(model, adjacency, i->first);

			std::vector< uint32_t > edgeNeighborTriangleIds;
			triangleEdgeNeighbors(model, adjacency, i->first, edgeNeighborTriangleIds);
			for (std::vector< uint32_t >::iterator j = edgeNeighborTriangleIds.begin(); j != edgeNeighborTriangleIds.end(); ++j)
				errors[*j] = triangleVolumeError(model, adjacency, *j);

			std::vector< std::pair< uint32_t, uint32_t > > singleNeighborTriangleIds;
			triangleSingleVertexNeighbors(model, i->first, singleNeighborTriangleIds);
			for (std::vector< std::pair< uint32_t, uint32_t > >::iterator j = singleNeighborTriangleIds.begin(); j != singleNeighborTriangleIds.end(); ++j)
				errors[j->first] = triangleVolumeError(model, adjacency, j->first);
		}
	}

	return true;
}

	}
}
