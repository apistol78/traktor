#include <numeric>
#include "Core/Math/TransformPath.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Database/Database.h"
#include "Model/Model.h"
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

Ref< model::Model > ExtrudeShapeLayerData::createModel(db::Database* database, const TransformPath& path) const
{
	// Read material from database.
	Ref< const model::Material > material = database->getObjectReadOnly< model::Material >(m_material);
	if (!material)
		return nullptr;

	const auto& keys = path.getKeys();
	if (keys.size() < 2)
		return nullptr;

	const uint32_t nsegments = (uint32_t)(keys.size() - 1);
	AlignedVector< uint32_t > stepsPerSegment;

	// Calculate number of steps per segment.
	for (uint32_t i = 0; i < nsegments; ++i)
	{
		const auto& k0 = keys[i];
		const auto& k1 = keys[i + 1];

		Scalar err(0.0f);
		for (uint32_t j = 0; j < 10; ++j)
		{
			float t = (float)j / (10 - 1);

			Vector4 pl = lerp(k0.position, k1.position, Scalar(t));
			Vector4 pc = path.evaluate(k0.T * (1.0f - t) + k1.T * t).transform().translation();

			err += (pl - pc).xyz0().length();
		}
		err /= Scalar(10.0f);

		stepsPerSegment.push_back(
			std::max< uint32_t >(
				(uint32_t)(err * m_detail),
				1
			)
		);
	}

	int32_t nsteps = std::accumulate(stepsPerSegment.begin(), stepsPerSegment.end(), 0);
	if (nsteps <= 1)
		return nullptr;

	// Extrude shape.
	AlignedVector< Vector2 > shape;
	shape.push_back(Vector2( 5.0f, -0.3f));
	shape.push_back(Vector2( 4.0f, 0.0f));
	shape.push_back(Vector2(-4.0f, 0.0f));
	shape.push_back(Vector2(-5.0f, -0.3f));

	AlignedVector< Vector2 > normals;
	normals.push_back(Vector2(0.0f, 1.0f));
	normals.push_back(Vector2(0.0f, 1.0f));
	normals.push_back(Vector2(0.0f, 1.0f));
	normals.push_back(Vector2(0.0f, 1.0f));

	const bool closed = false;

	const float minX = -4.0f;
	const float maxX = 4.0f;

	uint32_t nedges = (uint32_t)shape.size();
	uint32_t nvertices = nsteps * nedges;
	uint32_t ntriangles = (nsteps - 1) * nedges * 2;

	Ref< model::Model > outputModel = new model::Model();
	outputModel->addUniqueTexCoordChannel(L"UVMap");
	outputModel->addMaterial(*material);

	// Create vertices.
	Vector4 lastPosition = path.evaluate(0.0f).position;
	uint32_t baseStep = 0;
	float travel = 0.0f;

	for (uint32_t segment = 0; segment < nsegments; ++segment)
	{
		for (uint32_t step = 0; step < stepsPerSegment[segment]; ++step)
		{
			const float at = (float)(baseStep + step) / (nsteps - 1);

			auto v = path.evaluate(at);
			Matrix44 T = v.transform().toMatrix44();

			if (m_automaticOrientation)
			{
				const float c_atDelta = 0.001f;
				Transform Tp = path.evaluate(std::max(at - c_atDelta, 0.0f)).transform();
				Transform Tn = path.evaluate(std::min(at + c_atDelta, 1.0f)).transform();
				T = lookAt(Tp.translation().xyz1(), Tn.translation().xyz1()).inverse();
			}

			for (uint32_t i = 0; i < nedges; ++i)
			{
				Vector2 p = shape[i] * v.values[0];
				Vector2 n = normals[i];

				Vector4 ep = T * Vector4(p.x, p.y, 0.0f, 1.0f);
				Vector4 en = T * Vector4(n.x, n.y, 0.0f, 0.0f);

				uint32_t vp = outputModel->addPosition(ep);
				uint32_t vn = outputModel->addNormal(en);
				uint32_t vt = outputModel->addTexCoord(Vector2(
					(shape[i].x - minX) / (maxX - minX),
					travel
				));

				outputModel->addVertex(model::Vertex(vp, vn, vt));
			}

			travel += (T.translation() - lastPosition).xyz0().length();
			lastPosition = T.translation();
		}
		baseStep += stepsPerSegment[segment];
	}

	// Create polygons.
	for (int32_t ring = 0; ring < nsteps - 1; ++ring)
	{
		const uint32_t b0 = ring * nedges;
		const uint32_t b1 = (ring + 1) * nedges;

		for (uint32_t i = 0; i < nedges; ++i)
		{
			uint32_t a = i;
			uint32_t b = (i + 1) % nedges;

			outputModel->addPolygon(model::Polygon(
				0,
				b0 + b,
				b0 + a,
				b1 + a,
				b1 + b
			));
		}
	}

	return outputModel;
}

void ExtrudeShapeLayerData::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"material", m_material, AttributeType(type_of< model::Material >()));
	s >> Member< bool >(L"automaticOrientation", m_automaticOrientation);
	s >> Member< float >(L"detail", m_detail);
}

	}
}
