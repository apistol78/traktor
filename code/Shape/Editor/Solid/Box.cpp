#include "Core/Math/Aabb3.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Model/Model.h"
#include "Shape/Editor/Solid/Box.h"

namespace traktor
{
	namespace shape
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.Box", 0, Box, IShape)

Box::Box()
:	m_extent(1.0f, 1.0f, 1.0f)
{
}

Ref< model::Model > Box::createModel() const
{
	const Scalar two(2.0f);

	Vector4 vertices[8];
	Aabb3(-m_extent / two, m_extent / two).getExtents(vertices);

	const Vector4* normals = Aabb3::getNormals();

	Ref< model::Model > m = new model::Model();
	
	uint32_t materials[] =
	{
		m->addMaterial(model::Material(L"Roof", Color4f(1.0f, 0.5f, 0.5f, 1.0f))),
		m->addMaterial(model::Material(L"Wall", Color4f(0.5f, 1.0f, 0.5f, 1.0f))),
		m->addMaterial(model::Material(L"Floor", Color4f(0.5f, 0.5f, 1.0f, 1.0f)))
	};

	m->addUniqueTexCoordChannel(L"UVMap");

	for (uint32_t i = 0; i < 6; ++i)
	{
		const int* face = Aabb3::getFaces() + i * 4;

        Vector4 fu, fv;
		switch (majorAxis3(normals[i]))
		{
		case 0:
			fu = Vector4(0.0f, 0.0f, 1.0f);
			fv = Vector4(0.0f, 1.0f, 0.0f);
			break;
		case 1:
			fu = Vector4(1.0f, 0.0f, 0.0f);
			fv = Vector4(0.0f, 0.0f, 1.0f);
			break;
		case 2:
			fu = Vector4(1.0f, 0.0f, 0.0f);
			fv = Vector4(0.0f, 1.0f, 0.0f);
			break;
		}

		uint32_t v[4];
		uint32_t n = m->addUniqueNormal(normals[i]);

		for (int32_t j = 0; j < 4; ++j)
		{
			const Vector4& P = vertices[face[3 - j]];

			model::Vertex vx;
			vx.setPosition(m->addUniquePosition(P));
			vx.setNormal(n);
			vx.setTexCoord(0, m->addUniqueTexCoord(Vector2(
				dot3(fu, P),
				dot3(fv, P)
			)));
			v[j] = m->addUniqueVertex(vx);
		}

		uint32_t material = materials[1];
		if (normals[i].y() > 0.5f)
			material = materials[2];
		else if (normals[i].y() < -0.5f)
			material = materials[0];

		model::Polygon pol(material, v[0], v[1], v[2], v[3]);
		pol.setNormal(n);
		m->addPolygon(pol);
	}

	return m;
}

void Box::createAnchors(AlignedVector< Vector4 >& outAnchors) const
{
}

void Box::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"extent", m_extent);
}

	}
}