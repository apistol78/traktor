#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Model/Model.h"
#include "Shape/Editor/Solid/Cylinder.h"

namespace traktor
{
	namespace shape
	{
		namespace
		{

struct CylinderMaterial { const wchar_t* name; Color4f color; } c_materials[] =
{
	{   L"+Y", Color4f(1.0f, 0.5f, 1.0f, 1.0f) },
	{   L"-Y", Color4f(1.0f, 1.0f, 0.5f, 1.0f) },
	{ L"Side", Color4f(0.5f, 1.0f, 1.0f, 1.0f) },
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.Cylinder", 0, Cylinder, IShape)

Cylinder::Cylinder()
:	m_length(1.0f)
,   m_radius(0.5f)
,   m_faces(16)
{
}

Ref< model::Model > Cylinder::createModel() const
{
	Ref< model::Model > m = new model::Model();

	uint32_t tc = m->addUniqueTexCoordChannel(L"UVMap");
	uint32_t npy = m->addUniqueNormal(Vector4(0.0f, 1.0f, 0.0f));
	uint32_t nny = m->addUniqueNormal(Vector4(0.0f, -1.0f, 0.0f));

	const float hl = m_length / 2.0f;

	for (int32_t i = 0; i < 3; ++i)
	{
		model::Material material;
		material.setName(c_materials[i].name);
		material.setColor(c_materials[i].color);
		material.setDiffuseMap(model::Material::Map(L"Texture", tc, true));
		m->addMaterial(material);
	}

	model::Polygon top;
	top.setMaterial(0);
	top.setNormal(npy);

	model::Polygon bottom;
	top.setMaterial(1);
	bottom.setNormal(nny);

	for (int32_t i = 0; i < m_faces; ++i)
	{
		float phi1 = TWO_PI * float(i) / m_faces;
		float phi2 = TWO_PI * float(i + 1) / m_faces;
		float x1 = sin(phi1) * m_radius;
		float z1 = cos(phi1) * m_radius;
		float x2 = sin(phi2) * m_radius;
		float z2 = cos(phi2) * m_radius;
		float u1 = float(i) / m_faces;
		float u2 = float(i + 1) / m_faces;

		// Top
		{
			model::Vertex vx;
			vx.setPosition(m->addUniquePosition(Vector4(x1, hl, z1, 1.0f)));
			vx.setNormal(npy);
			vx.setTexCoord(tc, m->addUniqueTexCoord(Vector2(x1, z1)));
			top.addVertex(m->addUniqueVertex(vx));
		}

		// Bottom
		{
			model::Vertex vx;
			vx.setPosition(m->addUniquePosition(Vector4(x1, -hl, z1, 1.0f)));
			vx.setNormal(nny);
			vx.setTexCoord(tc, m->addUniqueTexCoord(Vector2(x1, z1)));
			bottom.addVertex(m->addUniqueVertex(vx));
		}

		// Side
		{
			Vector4 n1 = Vector4(x1, 0.0f, z1).normalized();
			Vector4 n2 = Vector4(x2, 0.0f, z2).normalized();
			Vector4 ns = (n1 + n2).normalized();

			model::Polygon side;
			side.setMaterial(2);
			side.setNormal(m->addUniqueNormal(ns));

			model::Vertex vx;
			vx.setPosition(m->addUniquePosition(Vector4(x1, hl, z1, 1.0f)));
			vx.setNormal(m->addUniqueNormal(n1));
			vx.setTexCoord(tc, m->addUniqueTexCoord(Vector2(u1, 0.0f)));
			side.addVertex(m->addUniqueVertex(vx));

			vx.setPosition(m->addUniquePosition(Vector4(x2, hl, z2, 1.0f)));
			vx.setNormal(m->addUniqueNormal(n1));
			vx.setTexCoord(tc, m->addUniqueTexCoord(Vector2(u2, 0.0f)));
			side.addVertex(m->addUniqueVertex(vx));

			vx.setPosition(m->addUniquePosition(Vector4(x2, -hl, z2, 1.0f)));
			vx.setNormal(m->addUniqueNormal(n1));
			vx.setTexCoord(tc, m->addUniqueTexCoord(Vector2(u2, 1.0f)));
			side.addVertex(m->addUniqueVertex(vx));

			vx.setPosition(m->addUniquePosition(Vector4(x1, -hl, z1, 1.0f)));
			vx.setNormal(m->addUniqueNormal(n1));
			vx.setTexCoord(tc, m->addUniqueTexCoord(Vector2(u1, 1.0f)));
			side.addVertex(m->addUniqueVertex(vx));

			m->addPolygon(side);
		}
	}

	top.flipWinding();

	m->addPolygon(top);
	m->addPolygon(bottom);

	return m;
}

void Cylinder::createAnchors(AlignedVector< Vector4 >& outAnchors) const
{
}

void Cylinder::serialize(ISerializer& s)
{
	s >> Member< float >(L"length", m_length, AttributeRange(0.0f));
    s >> Member< float >(L"radius", m_radius, AttributeRange(0.0f));
    s >> Member< int32_t >(L"faces", m_faces, AttributeRange(3));
}

	}
}