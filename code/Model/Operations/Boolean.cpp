/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/BspTree.h"
#include "Core/Math/Const.h"
#include "Core/Math/Winding3.h"
#include "Model/Model.h"
#include "Model/Operations/Boolean.h"
#include "Model/Operations/MergeModel.h"

namespace traktor::model
{
	namespace
	{

Vector4 packTexCoord(const Vector2& texCoord)
{
	return Vector4(texCoord.x, texCoord.y, 0.0f, 0.0f);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.Boolean", Boolean, IModelOperation)

Boolean::Boolean(
	const Model& modelA, const Transform& modelTransformA,
	const Model& modelB, const Transform& modelTransformB,
	BooleanOperation operation
)
:	m_modelA(modelA)
,	m_modelB(modelB)
,	m_modelTransformA(modelTransformA)
,	m_modelTransformB(modelTransformB)
,	m_operation(operation)
{
}

bool Boolean::apply(Model& model) const
{
	model.clear(Model::CfAll);

	if (m_modelA.getPolygonCount() == 0)
	{
		model.apply(MergeModel(m_modelB, m_modelTransformB, 0.001f));
		return true;
	}

	if (m_modelB.getPolygonCount() == 0)
	{
		model.apply(MergeModel(m_modelA, m_modelTransformA, 0.001f));
		return true;
	}

	SmallMap< uint32_t, uint32_t > materialMap;

	for (uint32_t i = 0; i < m_modelA.getMaterialCount(); ++i)
		materialMap[i] = model.addUniqueMaterial(m_modelA.getMaterial(i));

	const uint32_t materialOffset = m_modelA.getMaterialCount();

	for (uint32_t i = 0; i < m_modelB.getMaterialCount(); ++i)
		materialMap[materialOffset + i] = model.addUniqueMaterial(m_modelB.getMaterial(i));

	// \tbd Assume one shared channel.
	model.addUniqueTexCoordChannel(L"UVMap");

	AlignedVector< BspPolygon > polygonsA;
	for (const auto& polygon : m_modelA.getPolygons())
	{
		auto& bp = polygonsA.push_back();
		bp.setIndex(materialMap[polygon.getMaterial()]);
		for (const auto vertex : polygon.getVertices())
		{
			const auto& vx = m_modelA.getVertex(vertex);
			const Vector4 normal = m_modelA.getNormal(vx.getNormal() != c_InvalidIndex ? vx.getNormal() : polygon.getNormal());
			bp.addVertex(
				m_modelTransformA * m_modelA.getPosition(vx.getPosition()),
				m_modelTransformA * normal,
				packTexCoord(m_modelA.getTexCoord(vx.getTexCoord(0)))
			);
		}
		if (!bp.calculatePlane())
			polygonsA.pop_back();
	}
	if (polygonsA.size() != m_modelA.getPolygonCount())
		log::error << L"Unable to generate " << (m_modelA.getPolygonCount() - polygonsA.size()) << L" planes; polygons in model A skipped." << Endl;

	AlignedVector< BspPolygon > polygonsB;
	for (const auto& polygon : m_modelB.getPolygons())
	{
		auto& bp = polygonsB.push_back();
		bp.setIndex(materialMap[materialOffset + polygon.getMaterial()]);
		for (const auto vertex : polygon.getVertices())
		{
			const auto& vx = m_modelB.getVertex(vertex);
			const Vector4 normal = m_modelB.getNormal(vx.getNormal() != c_InvalidIndex ? vx.getNormal() : polygon.getNormal());
			bp.addVertex(
				m_modelTransformB * m_modelB.getPosition(vx.getPosition()),
				m_modelTransformB * normal,
				packTexCoord(m_modelB.getTexCoord(vx.getTexCoord(0)))
			);
		}
		if (!bp.calculatePlane())
			polygonsB.pop_back();
	}
	if (polygonsB.size() != m_modelB.getPolygonCount())
		log::error << L"Unable to generate " << (m_modelB.getPolygonCount() - polygonsB.size()) << L" planes; polygons in model B skipped." << Endl;

	const bool fast = true;

	BspNode A;
	A.build(polygonsA, fast);

	BspNode B;
	B.build(polygonsB, fast);

	BspNode C;
	switch (m_operation)
	{
	case BoUnion:
		C = A.unioon(B, fast);
		break;

	case BoIntersection:
		C = A.intersection(B, fast);
		break;

	case BoDifference:
		C = A.difference(B, fast);
		break;
	}

	auto polygons = C.allPolygons();

	model.reservePolygons((uint32_t)polygons.size());
	for (const auto& bp : polygons)
	{
		const Vector4 normal = bp.getPlane().normal();
		const uint32_t n = model.addUniqueNormal(normal);

		Polygon polygon;
		polygon.setMaterial((uint32_t)bp.getIndex());
		polygon.setNormal(n);
		for (const auto& p : bp.getVertices())
		{
			float uv[4];
			p.attributes[1].storeUnaligned(uv);

			Vector4 vertexNormal = p.attributes[0].normalized();
			if (dot3(vertexNormal, normal) < 0.0f)
				vertexNormal = -vertexNormal;

			Vertex vertex;
			vertex.setPosition(model.addUniquePosition(p.position));
			vertex.setNormal(model.addUniqueNormal(vertexNormal));
			vertex.setTexCoord(0, model.addUniqueTexCoord(Vector2(uv[0], uv[1])));
			polygon.addVertex(model.addUniqueVertex(vertex));
		}
		model.addPolygon(polygon);
	}

	return true;
}

}
