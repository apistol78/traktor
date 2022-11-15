/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <limits>
#include "Core/Math/BspTree.h"

namespace traktor
{
	namespace
	{
	
const float c_thickness = 0.001f;

	}

BspPolygon::BspPolygon(intptr_t index, const vertices_t& vertices)
:	m_index(index)
,	m_vertices(vertices)
{
}

void BspPolygon::setIndex(intptr_t index)
{
	m_index = index;
}

void BspPolygon::addVertex(const BspVertex& vertex)
{
	m_vertices.push_back(vertex);
}

void BspPolygon::addVertex(const Vector4& vertex, const Vector4& attr0, const Vector4& attr1, const Vector4& attr2, const Vector4& attr3)
{
	m_vertices.push_back({ vertex, { attr0, attr1, attr2, attr3 } });
}

void BspPolygon::setPlane(const Plane& plane)
{
	m_plane = plane;
}

bool BspPolygon::calculatePlane()
{
	Winding3 w((uint32_t)m_vertices.size());
	for (uint32_t i = 0; i < m_vertices.size(); ++i)
		w[i] = m_vertices[i].position;
	return w.getPlane(m_plane);
}

void BspPolygon::flip()
{
	std::reverse(m_vertices.begin(), m_vertices.end());
	m_plane = Plane(
        -m_plane.normal(),
        -m_plane.distance()
    );
}

int32_t BspPolygon::classify(const Plane& plane) const
{
	int32_t side[2] = { 0, 0 };
	for (size_t i = 0; i < m_vertices.size(); ++i)
	{
		Scalar d = plane.distance(m_vertices[i].position);
		if (d > c_thickness)
			side[0]++;
		else if (d < -c_thickness)
			side[1]++;
	}
	if (side[0] && !side[1])
		return Front;
	else if (!side[0] && side[1])
		return Back;
	else if (!side[0] && !side[1])
		return Coplanar;
	else
		return Span;
}

void BspPolygon::split(const Plane& plane, AlignedVector< BspPolygon >& outCoplanarFront, AlignedVector< BspPolygon >& outCoplanarBack, AlignedVector< BspPolygon >& outFront, AlignedVector< BspPolygon >& outBack) const
{
	const int32_t cl = classify(plane);
	if (cl == Front)
		outFront.push_back(*this);
	else if (cl == Back)
		outBack.push_back(*this);
	else if (cl == Coplanar)
	{
		if (dot3(m_plane.normal(), plane.normal()) >= 0.0f)
			outCoplanarFront.push_back(*this);
		else
			outCoplanarBack.push_back(*this);
	}
	else	// Span
	{
		BspPolygon& fp = outFront.push_back();
		BspPolygon& bp = outBack.push_back();

		fp.setIndex(m_index);
		fp.setPlane(m_plane);

		bp.setIndex(m_index);
		bp.setPlane(m_plane);

		for (size_t i = 0, j = m_vertices.size() - 1; i < m_vertices.size(); j = i++)
		{
			const BspVertex& a = m_vertices[i];
			const BspVertex& b = m_vertices[j];

			const Scalar da = plane.distance(a.position);
			const Scalar db = plane.distance(b.position);

			if (
				(da < -c_thickness && db > -c_thickness) ||
				(da >  c_thickness && db <  c_thickness)
			)
			{
				Scalar k;
				plane.segmentIntersection(a.position, b.position, k);

				BspVertex p;
				p.position = lerp(a.position, b.position, k);
				p.attributes[0] = lerp(a.attributes[0], b.attributes[0], k);
				p.attributes[1] = lerp(a.attributes[1], b.attributes[1], k);
				p.attributes[2] = lerp(a.attributes[2], b.attributes[2], k);
				p.attributes[3] = lerp(a.attributes[3], b.attributes[3], k);

				fp.addVertex(p);
				bp.addVertex(p);
			}

			if (da >= c_thickness)
				fp.addVertex(a);
			if (da <= -c_thickness)
				bp.addVertex(a);
		}

		if (fp.getVertices().size() < 3)
		{
			bp = *this;
			outFront.pop_back();
		}
		if (bp.getVertices().size() < 3)
		{
			fp = *this;
			outBack.pop_back();
		}
	}
}

BspNode::BspNode(const BspNode& node)
:   m_front(nullptr)
,   m_back(nullptr)
{
	m_plane = node.m_plane;
	m_polygons = node.m_polygons;

	if (node.m_front)
		m_front = node.m_front->clone();
	if (node.m_back)
		m_back = node.m_back->clone();
}

BspNode::BspNode(BspNode&& node)
:   m_front(nullptr)
,   m_back(nullptr)
{
	m_plane = node.m_plane;
	m_polygons = std::move(node.m_polygons);
	m_front = node.m_front;
	node.m_front = nullptr;
	m_back = node.m_back;
	node.m_back = nullptr;
}

BspNode::~BspNode()
{
    delete m_back;
    delete m_front;
}

void BspNode::invert()
{
	m_plane = Plane(
        -m_plane.normal(),
        -m_plane.distance()
    );

	for (auto& polygon : m_polygons)
		polygon.flip();

	if (m_front)
		m_front->invert();
	if (m_back)
		m_back->invert();

	std::swap(m_front, m_back);
}

AlignedVector< BspPolygon > BspNode::clip(const AlignedVector< BspPolygon >& polygons) const
{
	AlignedVector< BspPolygon > front;
	AlignedVector< BspPolygon > back;

	for (const auto& polygon : polygons)
	{
		polygon.split(
			m_plane,
			front,	// coplanar front
			back,	// coplanar back
			front,	// front
			back	// back
		);
	}

	if (m_front)
		front = m_front->clip(front);
	if (m_back)
		back = m_back->clip(back);
	else
		back.resize(0);

	front.insert(front.end(), back.begin(), back.end());
	return front;
}

void BspNode::clip(const BspNode& other)
{
	m_polygons = other.clip(m_polygons);
	if (m_front)
		m_front->clip(other);
	if (m_back)
		m_back->clip(other);	
}

void BspNode::build(const AlignedVector< BspPolygon >& polygons, bool fast)
{
	if (polygons.empty())
		return;

	size_t index = std::numeric_limits< size_t >::max();

	if (!m_front && !m_back)
	{
		if (!fast)
		{
			int32_t minSpan = std::numeric_limits< int32_t >::max();
			size_t minSpanIndex = 0;

			while (minSpan > 0 && index < polygons.size())
			{
				Plane plane = polygons[index].getPlane();

				int32_t span = 0;
				for (size_t i = 0; i < polygons.size(); ++i)
				{
					if (i == index)
						continue;
					if (polygons[i].classify(plane) == BspPolygon::Span)
						++span;
				}

				if (span < minSpan)
				{
					minSpan = span;
					minSpanIndex = index;
				}
			}

			index = minSpanIndex;
		}
		else
			index = 0;

		m_plane = polygons[index].getPlane();
		m_polygons.push_back(polygons[index]);
	}

	AlignedVector< BspPolygon > front;
	AlignedVector< BspPolygon > back;

	for (size_t i = 0; i < polygons.size(); ++i)
	{
		if (i == index)
			continue;
		polygons[i].split(
			m_plane,
			m_polygons,	// coplanar front
			m_polygons,	// coplanar back
			front,	// front
			back	// back			
		);
	}

	if (!front.empty())
	{
		if (!m_front)
			m_front = new BspNode();
		m_front->build(front, fast);
	}
	if (!back.empty())
	{
		if (!m_back)
			m_back = new BspNode();
		m_back->build(back, fast);
	}
}

AlignedVector< BspPolygon > BspNode::allPolygons() const
{
	AlignedVector< BspPolygon > polygons = m_polygons;
	if (m_front)
	{
		AlignedVector< BspPolygon > front = m_front->allPolygons();
		polygons.insert(polygons.end(), front.begin(), front.end());
	}
	if (m_back)
	{
		AlignedVector< BspPolygon > back = m_back->allPolygons();
		polygons.insert(polygons.end(), back.begin(), back.end());
	}
	return polygons;
}

BspNode BspNode::unioon(const BspNode& other, bool fast) const
{
	BspNode A = *this;
	BspNode B = other;

	// A | B
	A.clip(B);
	B.clip(A);
	B.invert();
	B.clip(A);
	B.invert();
	A.build(B.allPolygons(), fast);

	return A;
}

BspNode BspNode::intersection(const BspNode& other, bool fast) const
{
	BspNode A = *this;
	BspNode B = other;

	// A & B == ~(~A | ~B)
	A.invert();
	B.clip(A);
	B.invert();
	A.clip(B);
	B.clip(A);
	A.build(B.allPolygons(), fast);
	A.invert();

	return A;
}

BspNode BspNode::difference(const BspNode& other, bool fast) const
{
	BspNode A = *this;
	BspNode B = other;

	// A - B == ~(~A | B)
	A.invert();
	A.clip(B);
	B.clip(A);
	B.invert();
	B.clip(A);
	B.invert();
	A.build(B.allPolygons(), fast);
	A.invert();

	return A;
}

BspNode& BspNode::operator = (const BspNode& node)
{
	delete m_front;
	delete m_back;

	m_plane = node.m_plane;
	m_polygons = node.m_polygons;
	m_front = nullptr;
	m_back = nullptr;

	if (node.m_front)
		m_front = node.m_front->clone();
	if (node.m_back)
		m_back = node.m_back->clone();

	return *this;
}

BspNode& BspNode::operator = (BspNode&& node)
{
	delete m_front;
	delete m_back;

	m_plane = node.m_plane;
	m_polygons = std::move(node.m_polygons);
	m_front = node.m_front;
	node.m_front = nullptr;
	m_back = node.m_back;
	node.m_back = nullptr;
	
	return *this;
}

BspNode* BspNode::clone() const
{
	BspNode* copy = new BspNode();

	copy->m_plane = m_plane;
	copy->m_polygons = m_polygons;

	if (m_front)
		copy->m_front = m_front->clone();
	if (m_back)
		copy->m_back = m_back->clone();

	return copy;
}

}
