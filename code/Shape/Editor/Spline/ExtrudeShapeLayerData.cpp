#include <numeric>
#include "Core/Io/FileSystem.h"
#include "Core/Math/TransformPath.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Database/Database.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Model/Operations/Triangulate.h"
#include "Shape/Editor/Spline/ExtrudeShapeLayer.h"
#include "Shape/Editor/Spline/ExtrudeShapeLayerData.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.ExtrudeShapeLayerData", 1, ExtrudeShapeLayerData, SplineLayerComponentData)

ExtrudeShapeLayerData::ExtrudeShapeLayerData()
:	m_automaticOrientation(false)
,	m_detail(10.0f)
{
}

Ref< SplineLayerComponent > ExtrudeShapeLayerData::createComponent(db::Database* database) const
{
	return new ExtrudeShapeLayer(this);
}

Ref< model::Model > ExtrudeShapeLayerData::createModel(db::Database* database, const std::wstring& assetPath, const TransformPath& path) const
{
	if (!m_model)
	{
		// Read extrude mesh asset from database.
		Ref< const mesh::MeshAsset > meshAsset = database->getObjectReadOnly< mesh::MeshAsset >(m_mesh);
		if (!meshAsset)
			return nullptr;

		// Read model specified by mesh asset.
		Path filePath = FileSystem::getInstance().getAbsolutePath(Path(assetPath) + Path(meshAsset->getFileName()));
		m_model = model::ModelFormat::readAny(filePath);
		if (!m_model)
			return nullptr;

		// Only triangles; this method must only return triangle meshes.
		model::Triangulate().apply(*m_model);
	}

	const auto& keys = path.getKeys();
	if (keys.size() < 2)
		return nullptr;

	// Get all unique Z positions in model.
	SmallSet< float > steps;
	float stepMin = std::numeric_limits< float >::max();
	float stepMax = -std::numeric_limits< float >::max();
	for (const auto& position : m_model->getPositions())
	{
		steps.insert(position.z());
		stepMin = std::min< float >(stepMin, position.z());
		stepMax = std::max< float >(stepMin, position.z());
	}
	if (steps.empty())
		return nullptr;

	float stepLength = stepMax - stepMin;

	// Determine geometric length of path.
	float pathLength = 0.0f;
	for (int32_t i = 0; i < 100; ++i)
	{
		float t0 = (float)i / 100.0f;
		float t1 = (float)(i + 1) / 100.0f;

		t0 = clamp(t0, 0.0f, 1.0f);
		t1 = clamp(t1, 0.0f, 1.0f);

		auto v0 = path.evaluate(t0);
		auto v1 = path.evaluate(t1);

		Vector4 p0 = v0.transform().translation();
		Vector4 p1 = v1.transform().translation();

		pathLength += (p1 - p0).length();
	}

	// Extrude shape.
	Ref< model::Model > outputModel = new model::Model();
	outputModel->setMaterials(m_model->getMaterials());
	outputModel->setTexCoords(m_model->getTexCoords());
	outputModel->setTexCoordChannels(m_model->getTexCoordChannels());

	const int32_t nrepeats = (int32_t)(pathLength / stepLength) + 1;
	for (int32_t i = 0; i < nrepeats; ++i)
	{
		float at = (float)i / nrepeats;

		uint32_t vertexBase = outputModel->getVertexCount();

		for (const auto& vertex : m_model->getVertices())
		{
			Vector4 p = m_model->getPosition(vertex.getPosition());
			Vector4 n = m_model->getNormal(vertex.getNormal());

			Matrix44 Tc = translate(0.0f, 0.0f, p.z());

			float ats = at + ((p.z() - stepMin) / stepLength) * (1.0f / nrepeats);
			ats = clamp(ats, 0.0f, 1.0f);

			auto v = path.evaluate(ats);
			Matrix44 T = v.transform().toMatrix44();

			if (m_automaticOrientation)
			{
				const float c_atDelta = 0.001f;
				Transform Tp = path.evaluate(std::max(ats - c_atDelta, 0.0f)).transform();
				Transform Tn = path.evaluate(std::min(ats + c_atDelta, 1.0f)).transform();
				T = lookAt(Tp.translation().xyz1(), Tn.translation().xyz1()).inverse();
			}

			model::Vertex outputVertex;
			outputVertex.setPosition(outputModel->addPosition(T * Tc.inverse() * p.xyz1()));
			outputVertex.setNormal(outputModel->addNormal(T * n.xyz0()));
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

	return outputModel;
}

void ExtrudeShapeLayerData::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"mesh", m_mesh, AttributeType(type_of< mesh::MeshAsset >()));
	s >> Member< bool >(L"automaticOrientation", m_automaticOrientation);
	s >> Member< float >(L"detail", m_detail);
}

	}
}
