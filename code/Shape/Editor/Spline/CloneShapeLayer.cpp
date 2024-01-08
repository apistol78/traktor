/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/TransformPath.h"
#include "Core/Settings/PropertyObject.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Shape/Editor/Spline/CloneShapeLayer.h"
#include "Shape/Editor/Spline/CloneShapeLayerData.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.CloneShapeLayer", CloneShapeLayer, SplineLayerComponent)

CloneShapeLayer::CloneShapeLayer(
	const CloneShapeLayerData* data,
	mesh::MeshAsset* mesh,
	const model::Model* model
)
:	m_data(data)
,	m_mesh(mesh)
,	m_model(model)
{
}

void CloneShapeLayer::destroy()
{
}

void CloneShapeLayer::setOwner(world::Entity* owner)
{
}

void CloneShapeLayer::setTransform(const Transform& transform)
{
}

Aabb3 CloneShapeLayer::getBoundingBox() const
{
	return Aabb3();
}

void CloneShapeLayer::update(const world::UpdateParams& update)
{
}

Ref< model::Model > CloneShapeLayer::createModel(const TransformPath& path, bool closed, bool preview) const
{
	const auto& keys = path.getKeys();
	if (keys.size() < 2)
		return nullptr;

	// Determine geometric length of path.
	float pathLength = 0.0f;
	for (int32_t i = 0; i < 100; ++i)
	{
		const float t0 = (float)i / 100.0f;
		const float t1 = (float)(i + 1) / 100.0f;

		const auto v0 = path.evaluate(t0, closed);
		const auto v1 = path.evaluate(t1, closed);

		const Vector4 p0 = v0.transform().translation();
		const Vector4 p1 = v1.transform().translation();

		pathLength += (p1 - p0).length();
	}

	// Extrude shape.
	Ref< model::Model > outputModel = new model::Model();
	outputModel->setMaterials(m_model->getMaterials());
	outputModel->setTexCoords(m_model->getTexCoords());
	outputModel->setTexCoordChannels(m_model->getTexCoordChannels());

	const int32_t nrepeats = (int32_t)(pathLength / m_data->m_distance) + 1;
	for (int32_t i = 0; i < nrepeats; ++i)
	{
		const float at = (float)i / nrepeats;

		const uint32_t vertexBase = outputModel->getVertexCount();

		const auto v = path.evaluate(at, closed);
		Matrix44 T = v.transform().toMatrix44();

		if (m_data->m_automaticOrientation)
		{
			const Quaternion Qrot(T);

			const float c_atDelta = 0.001f;
			const Transform Tp = path.evaluate(at - c_atDelta, closed).transform();
			const Transform Tn = path.evaluate(at + c_atDelta, closed).transform();
			T = lookAt(Tp.translation().xyz1(), Tn.translation().xyz1()).inverse();

			T = T * rotateZ(Qrot.toEulerAngles().y());
		}

		for (const auto& vertex : m_model->getVertices())
		{
			const Vector4 p = m_model->getPosition(vertex.getPosition());
			const Vector4 n = m_model->getNormal(vertex.getNormal());

			model::Vertex outputVertex;
			outputVertex.setPosition(outputModel->addPosition(T * p.xyz1()));
			outputVertex.setNormal(outputModel->addNormal(T * n.xyz0()));
			outputVertex.setTexCoord(0, vertex.getTexCoord(0));
			outputModel->addVertex(outputVertex);
		}

		for (const auto& polygon : m_model->getPolygons())
		{
			model::Polygon outputPolygon;
			outputPolygon.setMaterial(polygon.getMaterial());
			for (auto id : polygon.getVertices())
				outputPolygon.addVertex(id + vertexBase);
			outputModel->addPolygon(outputPolygon);
		}
	}

	outputModel->setProperty< PropertyObject >(type_name(m_mesh), m_mesh);
	return outputModel;
}

}
