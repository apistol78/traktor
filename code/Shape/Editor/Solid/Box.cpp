/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Aabb3.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Model/Model.h"
#include "Shape/Editor/Solid/Box.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.shape.Box", 2, Box, IShape)

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
		material.setName(L"M_Box_" + toString(i));
		material.setColor(Color4f(1.0f, 1.0f, 1.0f, 1.0f));
		material.setRoughness(1.0f);

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
		pol.addVertex(vi[3]);
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
	if (s.getVersion() >= 1 && s.getVersion() < 2)
	{
		Guid materials[6];
		s >> MemberStaticArray< Guid, 6 >(L"materials", materials);
	}
}

	}
}