/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Range.h"
#include "Core/Math/TransformPath.h"
#include "Core/Settings/PropertyObject.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Shape/Editor/Spline/ExtrudeShapeLayer.h"
#include "Shape/Editor/Spline/ExtrudeShapeLayerData.h"

namespace traktor::shape
{
	namespace
	{

Range< float > calculateZRange(const model::Model& m)
{
	Range< float > rng(
		std::numeric_limits< float >::max(),
		-std::numeric_limits< float >::max()
	);
	for (const auto& position : m.getPositions())
	{
		rng.min = std::min< float >(rng.min, position.z());
		rng.max = std::max< float >(rng.max, position.z());
	}
	return rng;	
}

	}

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

Ref< model::Model > ExtrudeShapeLayer::createModel(const TransformPath& path_, bool closed, bool preview) const
{
	if (path_.size() < 2)
		return nullptr;

	const TransformPath path = path_.geometricNormalized(closed);
	const float pathLength = path.measureLength(closed);

	const Range< float > step = calculateZRange(*m_modelRepeat);
	const Matrix44 To = translate(m_data->m_offset, 0.0f, 0.0f);

	// Extrude shape.
	Ref< model::Model > outputModel = new model::Model();
	outputModel->setMaterials(m_modelRepeat->getMaterials());
	outputModel->setTexCoords(m_modelRepeat->getTexCoords());
	outputModel->setTexCoordChannels(m_modelRepeat->getTexCoordChannels());

	const int32_t nrepeats = (int32_t)(pathLength / step.delta()) + 1;

	outputModel->reservePositions(nrepeats * m_modelRepeat->getVertices().size());
	outputModel->reserveNormals(nrepeats * m_modelRepeat->getVertices().size());
	outputModel->reserveVertices(nrepeats * m_modelRepeat->getVertices().size());
	outputModel->reservePolygons(nrepeats * m_modelRepeat->getPolygons().size());

	for (int32_t i = 0; i < nrepeats; ++i)
	{
		const float at = (float)i / nrepeats;

		const uint32_t vertexBase = outputModel->getVertexCount();

		for (const auto& vertex : m_modelRepeat->getVertices())
		{
			const Vector4 p = m_modelRepeat->getPosition(vertex.getPosition());
			const Vector4 n = m_modelRepeat->getNormal(vertex.getNormal());

			const Matrix44 Tc = translate(0.0f, 0.0f, p.z());

			const float ats = at + ((p.z() - step.min) / step.delta()) * (1.0f / nrepeats);

			const auto v = path.evaluate(ats, closed);
			Matrix44 T = v.transform().toMatrix44();

			const float automaticOrientationWeight = v.values[1];
			if (m_data->m_automaticOrientation && automaticOrientationWeight > FUZZY_EPSILON)
			{
				const Quaternion Qrot(T);

				Matrix44 Tao = T;

				const float c_atDelta = 0.001f;
				const Transform Tp = path.evaluate(ats - c_atDelta, closed).transform();
				const Transform Tn = path.evaluate(ats + c_atDelta, closed).transform();
				Tao = lookAt(Tp.translation().xyz1(), Tn.translation().xyz1()).inverse();
				Tao = Tao * rotateZ(Qrot.toEulerAngles().y());

				if (automaticOrientationWeight < 1.0f)
				{
					const Quaternion QaoRot(Tao);
					const Quaternion Qr = lerp(Qrot, QaoRot, Scalar(automaticOrientationWeight));
					T = translate(Tao.translation()) * Qr.toMatrix44();
				}
				else
					T = Tao;
			}

			model::Vertex outputVertex;
			outputVertex.setPosition(outputModel->addPosition(T * Tc.inverse() * To * p.xyz1()));
			outputVertex.setNormal(outputModel->addNormal(T * n.xyz0()));
			outputVertex.setTexCoord(0, vertex.getTexCoord(0));
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
	if (!preview)
		model::CleanDuplicates(0.01f).apply(*outputModel);

	outputModel->setProperty< PropertyObject >(type_name(m_meshRepeat), m_meshRepeat);
	return outputModel;
}

}
