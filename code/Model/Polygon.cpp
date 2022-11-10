/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStaticVector.h"
#include "Model/Polygon.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.Polygon", 0, Polygon, ISerializable)

Polygon::Polygon(uint32_t material, uint32_t vertex1, uint32_t vertex2)
:	m_material(material)
,	m_vertices(2)
{
	m_vertices[0] = vertex1;
	m_vertices[1] = vertex2;
}

Polygon::Polygon(uint32_t material, uint32_t vertex1, uint32_t vertex2, uint32_t vertex3)
:	m_material(material)
,	m_vertices(3)
{
	m_vertices[0] = vertex1;
	m_vertices[1] = vertex2;
	m_vertices[2] = vertex3;
}

Polygon::Polygon(uint32_t material, uint32_t vertex1, uint32_t vertex2, uint32_t vertex3, uint32_t vertex4)
:	m_material(material)
,	m_vertices(4)
{
	m_vertices[0] = vertex1;
	m_vertices[1] = vertex2;
	m_vertices[2] = vertex3;
	m_vertices[3] = vertex4;
}

void Polygon::setMaterial(uint32_t material)
{
	m_material = material;
}

void Polygon::setNormal(uint32_t normal)
{
	m_normal = normal;
}

void Polygon::setSmoothGroup(uint32_t smoothGroup)
{
	m_smoothGroup = smoothGroup;
}

void Polygon::clearVertices()
{
	m_vertices.resize(0);
}

void Polygon::flipWinding()
{
	if (!m_vertices.empty())
		std::reverse(m_vertices.begin(), m_vertices.end());
}

void Polygon::addVertex(uint32_t vertex)
{
	m_vertices.push_back(vertex);
}

void Polygon::insertVertex(uint32_t index, uint32_t vertex)
{
	m_vertices.insert(m_vertices.begin() + (size_t)index, vertex);
}

void Polygon::setVertex(uint32_t index, uint32_t vertex)
{
	T_ASSERT(index < uint32_t(m_vertices.size()));
	m_vertices[index] = vertex;
}

void Polygon::setVertices(const vertices_t& vertices)
{
	m_vertices = vertices;
}

void Polygon::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"material", m_material);
	s >> Member< uint32_t >(L"normal", m_normal);
	s >> Member< uint32_t >(L"smoothGroup", m_smoothGroup);
	s >> MemberStaticVector< uint32_t, vertices_t::Capacity >(L"vertices", m_vertices);
}

bool Polygon::operator == (const Polygon& r) const
{
	return m_material == r.m_material && m_vertices == r.m_vertices;
}

	}
}
