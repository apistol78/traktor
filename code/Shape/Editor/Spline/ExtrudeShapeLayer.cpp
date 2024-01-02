/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/TransformPath.h"
#include "Core/Settings/PropertyObject.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Shape/Editor/Spline/ExtrudeShapeLayer.h"
#include "Shape/Editor/Spline/ExtrudeShapeLayerData.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.ExtrudeShapeLayer", ExtrudeShapeLayer, SplineLayerComponent)

ExtrudeShapeLayer::ExtrudeShapeLayer(
	const ExtrudeShapeLayerData* data,
	mesh::MeshAsset* meshStart, const model::Model* modelStart,
	mesh::MeshAsset* meshRepeat, const model::Model* modelRepeat,
	mesh::MeshAsset* meshEnd, const model::Model* modelEnd
)
:	m_data(data)
,	m_meshStart(meshStart)
,	m_modelStart(modelStart)
,	m_meshRepeat(meshRepeat)
,	m_modelRepeat(modelRepeat)
,	m_meshEnd(meshEnd)
,	m_modelEnd(modelEnd)
{
}

void ExtrudeShapeLayer::destroy()
{
}

void ExtrudeShapeLayer::setOwner(world::Entity* owner)
{
}

void ExtrudeShapeLayer::setTransform(const Transform& transform)
{
}

Aabb3 ExtrudeShapeLayer::getBoundingBox() const
{
	return Aabb3();
}

void ExtrudeShapeLayer::update(const world::UpdateParams& update)
{
}

Ref< model::Model > ExtrudeShapeLayer::createModel(const TransformPath& path) const
{
	const auto& keys = path.getKeys();
	if (keys.size() < 2)
		return nullptr;

	// Get all unique Z positions in model.
	SmallSet< float > steps;
	float stepMin = std::numeric_limits< float >::max();
	float stepMax = -std::numeric_limits< float >::max();
	for (const auto& position : m_modelRepeat->getPositions())
	{
		steps.insert(position.z());
		stepMin = std::min< float >(stepMin, position.z());
		stepMax = std::max< float >(stepMax, position.z());
	}
	if (steps.empty())
		return nullptr;

	const float stepLength = stepMax - stepMin;

	// Determine geometric length of path.
	float pathLength = 0.0f;
	for (int32_t i = 0; i < 100; ++i)
	{
		float t0 = (float)i / 100.0f;
		float t1 = (float)(i + 1) / 100.0f;

		t0 = clamp(t0, 0.0f, 1.0f);
		t1 = clamp(t1, 0.0f, 1.0f);

		const auto v0 = path.evaluate(t0);
		const auto v1 = path.evaluate(t1);

		const Vector4 p0 = v0.transform().translation();
		const Vector4 p1 = v1.transform().translation();

		pathLength += (p1 - p0).length();
	}

	// Extrude shape.
	Ref< model::Model > outputModel = new model::Model();
	outputModel->setMaterials(m_modelRepeat->getMaterials());
	outputModel->setTexCoords(m_modelRepeat->getTexCoords());
	outputModel->setTexCoordChannels(m_modelRepeat->getTexCoordChannels());

	const int32_t nrepeats = (int32_t)(pathLength / stepLength) + 1;
	for (int32_t i = 0; i < nrepeats; ++i)
	{
		const float at = (float)i / nrepeats;

		const uint32_t vertexBase = outputModel->getVertexCount();

		for (const auto& vertex : m_modelRepeat->getVertices())
		{
			const Vector4 p = m_modelRepeat->getPosition(vertex.getPosition());
			const Vector4 n = m_modelRepeat->getNormal(vertex.getNormal());

			const Matrix44 Tc = translate(0.0f, 0.0f, p.z());

			float ats = at + ((p.z() - stepMin) / stepLength) * (1.0f / nrepeats);
			ats = clamp(ats, 0.0f, 1.0f);

			const auto v = path.evaluate(ats);
			Matrix44 T = v.transform().toMatrix44();

			if (m_data->m_automaticOrientation)
			{
				const float c_atDelta = 0.001f;
				const Transform Tp = path.evaluate(std::max(ats - c_atDelta, 0.0f)).transform();
				const Transform Tn = path.evaluate(std::min(ats + c_atDelta, 1.0f)).transform();
				T = lookAt(Tp.translation().xyz1(), Tn.translation().xyz1()).inverse();
			}

			model::Vertex outputVertex;
			outputVertex.setPosition(outputModel->addPosition(T * Tc.inverse() * p.xyz1()));
			outputVertex.setNormal(outputModel->addNormal(T * n.xyz0()));
			outputModel->addVertex(outputVertex);
		}

		for (const auto& polygon : m_modelRepeat->getPolygons())
		{
			model::Polygon outputPolygon;
			outputPolygon.setMaterial(polygon.getMaterial());
			for (auto id : polygon.getVertices())
				outputPolygon.addVertex(id + vertexBase);
			outputModel->addPolygon(outputPolygon);
		}
	}

	// Cleanup since there are probably a lot of duplicates.
	model::CleanDuplicates(0.01f).apply(*outputModel);

	outputModel->setProperty< PropertyObject >(type_name(m_meshRepeat), m_meshRepeat);
	return outputModel;
}

}
