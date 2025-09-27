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
#include "Shape/Editor/Solid/Capsule.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.Capsule", 2, Capsule, IShape)

Capsule::Capsule()
:	m_radius(0.5f)
,	m_height(1.0f)
,   m_segments(24)
,   m_rings(12)
{
}

Ref< model::Model > Capsule::createModel() const
{
	Ref< model::Model > m = new model::Model();

	uint32_t tc = m->addUniqueTexCoordChannel(L"UVMap");

	model::Material material;
	material.setName(L"M_Capsule");
	material.setColor(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
	material.setRoughness(1.0f);
	uint32_t mi = m->addMaterial(material);

	const float halfHeight = m_height * 0.5f;
	const int32_t hemisphereRings = m_rings / 2;

	// Generate bottom hemisphere
	AlignedVector< uint32_t > bottomVertices;
	bottomVertices.resize((hemisphereRings + 1) * (m_segments + 1));

	for (int32_t ring = 0; ring <= hemisphereRings; ++ring)
	{
		const float phi = PI * 0.5f + (PI * 0.5f * float(ring) / hemisphereRings);
		const float y = cos(phi) * m_radius - halfHeight;
		const float ringRadius = sin(phi) * m_radius;

		for (int32_t segment = 0; segment <= m_segments; ++segment)
		{
			const float theta = TWO_PI * float(segment) / m_segments;
			const float x = cos(theta) * ringRadius;
			const float z = sin(theta) * ringRadius;

			const Vector4 position(x, y, z, 1.0f);
			const Vector4 normal = Vector4(x, y + halfHeight, z, 0.0f).normalized();
			const Vector2 texCoord(float(segment) / m_segments, float(ring) / hemisphereRings * 0.25f);

			model::Vertex vx;
			vx.setPosition(m->addUniquePosition(position));
			vx.setNormal(m->addUniqueNormal(normal));
			vx.setTexCoord(tc, m->addUniqueTexCoord(texCoord));

			bottomVertices[ring * (m_segments + 1) + segment] = m->addUniqueVertex(vx);
		}
	}

	// Generate bottom hemisphere triangles
	for (int32_t ring = 0; ring < hemisphereRings; ++ring)
	{
		for (int32_t segment = 0; segment < m_segments; ++segment)
		{
			const uint32_t current = ring * (m_segments + 1) + segment;
			const uint32_t next = current + m_segments + 1;

			// First triangle
			{
				model::Polygon pol;
				pol.setMaterial(mi);
				pol.addVertex(bottomVertices[current]);
				pol.addVertex(bottomVertices[next]);
				pol.addVertex(bottomVertices[current + 1]);
				m->addPolygon(pol);
			}

			// Second triangle
			{
				model::Polygon pol;
				pol.setMaterial(mi);
				pol.addVertex(bottomVertices[current + 1]);
				pol.addVertex(bottomVertices[next]);
				pol.addVertex(bottomVertices[next + 1]);
				m->addPolygon(pol);
			}
		}
	}

	// Generate cylinder sides (exactly like Cylinder implementation)
	for (int32_t i = 0; i < m_segments; ++i)
	{
		float phi1 = TWO_PI * float(i) / m_segments;
		float phi2 = TWO_PI * float(i + 1) / m_segments;
		float x1 = sin(phi1) * m_radius;  // Note: sin/cos swapped to match Cylinder
		float z1 = cos(phi1) * m_radius;
		float x2 = sin(phi2) * m_radius;
		float z2 = cos(phi2) * m_radius;
		float u1 = float(i) / m_segments;
		float u2 = float(i + 1) / m_segments;

		Vector4 n1 = Vector4(x1, 0.0f, z1).normalized();
		Vector4 n2 = Vector4(x2, 0.0f, z2).normalized();

		model::Polygon side;
		side.setMaterial(mi);

		model::Vertex vx;
		// Match exact order from Cylinder.cpp
		vx.setPosition(m->addUniquePosition(Vector4(x1, halfHeight, z1, 1.0f)));
		vx.setNormal(m->addUniqueNormal(n1));
		vx.setTexCoord(tc, m->addUniqueTexCoord(Vector2(u1, 0.75f)));
		side.addVertex(m->addUniqueVertex(vx));

		vx.setPosition(m->addUniquePosition(Vector4(x2, halfHeight, z2, 1.0f)));
		vx.setNormal(m->addUniqueNormal(n2));
		vx.setTexCoord(tc, m->addUniqueTexCoord(Vector2(u2, 0.75f)));
		side.addVertex(m->addUniqueVertex(vx));

		vx.setPosition(m->addUniquePosition(Vector4(x2, -halfHeight, z2, 1.0f)));
		vx.setNormal(m->addUniqueNormal(n2));
		vx.setTexCoord(tc, m->addUniqueTexCoord(Vector2(u2, 0.25f)));
		side.addVertex(m->addUniqueVertex(vx));

		vx.setPosition(m->addUniquePosition(Vector4(x1, -halfHeight, z1, 1.0f)));
		vx.setNormal(m->addUniqueNormal(n1));
		vx.setTexCoord(tc, m->addUniqueTexCoord(Vector2(u1, 0.25f)));
		side.addVertex(m->addUniqueVertex(vx));

		m->addPolygon(side);
	}

	// Generate top hemisphere
	AlignedVector< uint32_t > topVertices;
	topVertices.resize((hemisphereRings + 1) * (m_segments + 1));

	for (int32_t ring = 0; ring <= hemisphereRings; ++ring)
	{
		const float phi = PI * 0.5f * float(ring) / hemisphereRings;
		const float y = cos(phi) * m_radius + halfHeight;
		const float ringRadius = sin(phi) * m_radius;

		for (int32_t segment = 0; segment <= m_segments; ++segment)
		{
			const float theta = TWO_PI * float(segment) / m_segments;
			const float x = cos(theta) * ringRadius;
			const float z = sin(theta) * ringRadius;

			const Vector4 position(x, y, z, 1.0f);
			const Vector4 normal = Vector4(x, y - halfHeight, z, 0.0f).normalized();
			const Vector2 texCoord(float(segment) / m_segments, 0.75f + float(ring) / hemisphereRings * 0.25f);

			model::Vertex vx;
			vx.setPosition(m->addUniquePosition(position));
			vx.setNormal(m->addUniqueNormal(normal));
			vx.setTexCoord(tc, m->addUniqueTexCoord(texCoord));

			topVertices[ring * (m_segments + 1) + segment] = m->addUniqueVertex(vx);
		}
	}

	// Generate top hemisphere triangles
	for (int32_t ring = 0; ring < hemisphereRings; ++ring)
	{
		for (int32_t segment = 0; segment < m_segments; ++segment)
		{
			const uint32_t current = ring * (m_segments + 1) + segment;
			const uint32_t next = current + m_segments + 1;

			// First triangle
			{
				model::Polygon pol;
				pol.setMaterial(mi);
				pol.addVertex(topVertices[current]);
				pol.addVertex(topVertices[next]);
				pol.addVertex(topVertices[current + 1]);
				m->addPolygon(pol);
			}

			// Second triangle
			{
				model::Polygon pol;
				pol.setMaterial(mi);
				pol.addVertex(topVertices[current + 1]);
				pol.addVertex(topVertices[next]);
				pol.addVertex(topVertices[next + 1]);
				m->addPolygon(pol);
			}
		}
	}

	return m;
}

void Capsule::createAnchors(AlignedVector< Vector4 >& outAnchors) const
{
}

void Capsule::serialize(ISerializer& s)
{
	s >> Member< float >(L"radius", m_radius, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"height", m_height, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
    s >> Member< int32_t >(L"segments", m_segments, AttributeRange(3));
    s >> Member< int32_t >(L"rings", m_rings, AttributeRange(4)); // Minimum 4 for proper hemisphere generation
	if (s.getVersion() >= 1 && s.getVersion() < 2)
	{
		Guid materials[1];
		s >> MemberStaticArray< Guid, 1 >(L"materials", materials);
	}
}

	}
}