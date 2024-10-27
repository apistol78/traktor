/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Vector4.h"
#include "Model/Model.h"
#include "Model/Operations/CalculateNormals.h"

namespace traktor::model
{
	namespace
	{

bool findBaseIndex(const Model& model, const Polygon& polygon, uint32_t& outBaseIndex)
{
	outBaseIndex = c_InvalidIndex;

	const auto& vertices = polygon.getVertices();
	for (uint32_t i = 0; i < uint32_t(vertices.size()); ++i)
	{
		const Vertex* v[] =
		{
			&model.getVertex(vertices[i]),
			&model.getVertex(vertices[(i + 1) % vertices.size()]),
			&model.getVertex(vertices[(i + 2) % vertices.size()])
		};

		const Vector4 p[] =
		{
			model.getPosition(v[0]->getPosition()),
			model.getPosition(v[1]->getPosition()),
			model.getPosition(v[2]->getPosition())
		};

		const Vector4 ep[] = { p[2] - p[0], p[1] - p[0] };
		if (ep[0].length() > FUZZY_EPSILON && ep[1].length() > FUZZY_EPSILON && cross(ep[0], ep[1]).length() > FUZZY_EPSILON)
		{
			outBaseIndex = i;
			return true;
		}
	}

	return false;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.CalculateNormals", CalculateNormals, IModelOperation)

CalculateNormals::CalculateNormals(bool replaceExisting)
:	m_replaceExisting(replaceExisting)
{
}

bool CalculateNormals::apply(Model& model) const
{
	const AlignedVector< Polygon >& polygons = model.getPolygons();
	AlignedVector< Vector4 > polygonNormals;
	AlignedVector< Vector4 > positionNormals;
	uint32_t degenerated = 0;

	// Calculate tangent base for each polygon.
	polygonNormals.resize(polygons.size(), Vector4::zero());
	for (uint32_t i = 0; i < uint32_t(polygons.size()); ++i)
	{
		const Polygon& polygon = polygons[i];
		uint32_t baseIndex;

		if (polygon.getVertexCount() < 3)
			continue;

		if (!findBaseIndex(model, polygon, baseIndex))
		{
			++degenerated;
			continue;
		}

		const auto& vertices = polygon.getVertices();
		const Vertex* v[] =
		{
			&model.getVertex(vertices[baseIndex]),
			&model.getVertex(vertices[(baseIndex + 1) % vertices.size()]),
			&model.getVertex(vertices[(baseIndex + 2) % vertices.size()])
		};

		const Vector4 p[] =
		{
			model.getPosition(v[0]->getPosition()),
			model.getPosition(v[1]->getPosition()),
			model.getPosition(v[2]->getPosition())
		};

		Vector4 ep[] = { p[2] - p[0], p[1] - p[0] };
		T_ASSERT(ep[0].length() > FUZZY_EPSILON);
		T_ASSERT(ep[1].length() > FUZZY_EPSILON);

		ep[0] = ep[0].normalized();
		ep[1] = ep[1].normalized();

		polygonNormals[i] = cross(ep[0], ep[1]).normalized();
	}

	// Build new vertex normals.
	positionNormals.resize(model.getPositionCount(), Vector4::zero());
	for (uint32_t i = 0; i < (uint32_t)polygons.size(); ++i)
	{
		Polygon polygon = polygons[i];

		const auto& vertices = polygon.getVertices();
		for (auto vertex : vertices)
		{
			if (polygonNormals[i].length() > FUZZY_EPSILON)
			{
				const uint32_t positionId = model.getVertex(vertex).getPosition();
				positionNormals[positionId] += polygonNormals[i];
			}
		}

		if (m_replaceExisting || polygon.getNormal() == c_InvalidIndex)
			polygon.setNormal(model.addUniqueNormal(polygonNormals[i]));
		model.setPolygon(i, polygon);
	}

	// Normalize vertex tangent bases.
	for (auto& positionNormal : positionNormals)
	{
		if (positionNormal.length() > FUZZY_EPSILON)
			positionNormal.normalize();
	}

	// Update vertices.
	for (uint32_t i = 0; i < model.getVertexCount(); ++i)
	{
		Vertex vertex = model.getVertex(i);
		const Vector4& n = positionNormals[vertex.getPosition()];
		if (m_replaceExisting || vertex.getNormal() == c_InvalidIndex)
			vertex.setNormal(model.addUniqueNormal(n));
		model.setVertex(i, vertex);
	}

	return true;
}

}
