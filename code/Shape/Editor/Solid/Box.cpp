#include "Core/Math/Aabb3.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Model/Model.h"
#include "Shape/Editor/Solid/Box.h"

namespace traktor
{
	namespace shape
	{
		namespace
		{

struct BoxMaterial { const wchar_t* name; Color4f color; } c_materials[] =
{
	{ L"-Z", Color4f(1.0f, 0.5f, 0.5f, 1.0f) },
	{ L"+X", Color4f(0.5f, 1.0f, 0.5f, 1.0f) },
	{ L"+Z", Color4f(0.5f, 0.5f, 1.0f, 1.0f) },
	{ L"-X", Color4f(0.5f, 1.0f, 1.0f, 1.0f) },
	{ L"+Y", Color4f(1.0f, 0.5f, 1.0f, 1.0f) },
	{ L"-Y", Color4f(1.0f, 1.0f, 0.5f, 1.0f) }
};

		}
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.Box", 0, Box, IShape)

Box::Box()
:	m_extent(1.0f, 1.0f, 1.0f)
{
}

Ref< model::Model > Box::createModel() const
{
	Vector4 vertices[8];
	Aabb3(-m_extent / Scalar(2.0f), m_extent / Scalar(2.0f)).getExtents(vertices);

	Ref< model::Model > m = new model::Model();

	uint32_t tc = m->addUniqueTexCoordChannel(L"UVMap");

	for (uint32_t i = 0; i < 6; ++i)
	{
		const Vector4* normals = Aabb3::getNormals();
		const int* face = Aabb3::getFaces() + i * 4;

		model::Material material;
		material.setName(c_materials[i].name);
		material.setColor(c_materials[i].color);
		material.setDiffuseMap(model::Material::Map(L"Texture", tc, true));
		uint32_t mi = m->addMaterial(material);

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

		uint32_t vi[4];
		uint32_t ni = m->addUniqueNormal(normals[i]);

		for (int32_t j = 0; j < 4; ++j)
		{
			const Vector4& P = vertices[face[3 - j]];

			model::Vertex vx;
			vx.setPosition(m->addUniquePosition(P));
			vx.setNormal(ni);
			vx.setTexCoord(0, m->addUniqueTexCoord(Vector2(
				dot3(fu, P),
				dot3(fv, P)
			)));
			vi[j] = m->addUniqueVertex(vx);
		}

		model::Polygon pol;
		pol.setMaterial(mi);
		pol.setNormal(ni);
		pol.addVertex(vi[0]);
		pol.addVertex(vi[1]);
		pol.addVertex(vi[2]);
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