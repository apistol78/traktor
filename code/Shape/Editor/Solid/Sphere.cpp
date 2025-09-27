/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Model/Model.h"
#include "Shape/Editor/Solid/Sphere.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.Sphere", 2, Sphere, IShape)

Sphere::Sphere()
:	m_radius(0.5f)
,   m_segments(24)
,   m_rings(12)
{
	/*
	  - Low quality: 12 segments × 8 rings
	  - Medium quality: 24 segments × 12 rings
	  - High quality: 32 segments × 16 rings
	  - Very high quality: 48 segments × 24 rings
	*/
}

Ref< model::Model > Sphere::createModel() const
{
	Ref< model::Model > m = new model::Model();

	uint32_t tc = m->addUniqueTexCoordChannel(L"UVMap");

	model::Material material;
	material.setName(L"M_Sphere");
	material.setColor(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
	material.setRoughness(1.0f);
	uint32_t mi = m->addMaterial(material);

	// Generate vertices
	AlignedVector< uint32_t > vertexIndices;
	vertexIndices.resize((m_rings + 1) * (m_segments + 1));

	for (int32_t ring = 0; ring <= m_rings; ++ring)
	{
		const float phi = PI * float(ring) / m_rings;
		const float y = cos(phi) * m_radius;
		const float ringRadius = sin(phi) * m_radius;

		for (int32_t segment = 0; segment <= m_segments; ++segment)
		{
			const float theta = TWO_PI * float(segment) / m_segments;
			const float x = cos(theta) * ringRadius;
			const float z = sin(theta) * ringRadius;

			const Vector4 position(x, y, z, 1.0f);
			const Vector4 normal = position.normalized();
			const Vector2 texCoord(float(segment) / m_segments, float(ring) / m_rings);

			model::Vertex vx;
			vx.setPosition(m->addUniquePosition(position));
			vx.setNormal(m->addUniqueNormal(normal));
			vx.setTexCoord(tc, m->addUniqueTexCoord(texCoord));

			vertexIndices[ring * (m_segments + 1) + segment] = m->addUniqueVertex(vx);
		}
	}

	// Generate triangles
	for (int32_t ring = 0; ring < m_rings; ++ring)
	{
		for (int32_t segment = 0; segment < m_segments; ++segment)
		{
			const uint32_t current = ring * (m_segments + 1) + segment;
			const uint32_t next = current + m_segments + 1;

			// First triangle
			{
				model::Polygon pol;
				pol.setMaterial(mi);
				pol.addVertex(vertexIndices[current]);
				pol.addVertex(vertexIndices[next]);
				pol.addVertex(vertexIndices[current + 1]);
				m->addPolygon(pol);
			}

			// Second triangle
			{
				model::Polygon pol;
				pol.setMaterial(mi);
				pol.addVertex(vertexIndices[current + 1]);
				pol.addVertex(vertexIndices[next]);
				pol.addVertex(vertexIndices[next + 1]);
				m->addPolygon(pol);
			}
		}
	}

	return m;
}

void Sphere::createAnchors(AlignedVector< Vector4 >& outAnchors) const
{
}

void Sphere::serialize(ISerializer& s)
{
	s >> Member< float >(L"radius", m_radius, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
    s >> Member< int32_t >(L"segments", m_segments, AttributeRange(3));
    s >> Member< int32_t >(L"rings", m_rings, AttributeRange(3));
	if (s.getVersion() >= 1 && s.getVersion() < 2)
	{
		Guid materials[1];
		s >> MemberStaticArray< Guid, 1 >(L"materials", materials);
	}
}

	}
}