#include <algorithm>
#include "Model/Polygon.h"

namespace traktor
{
	namespace model
	{

Polygon::Polygon()
:	m_material(c_InvalidIndex)
,	m_normal(c_InvalidIndex)
,	m_smoothGroup(0)
{
}

Polygon::Polygon(uint32_t material, uint32_t vertex1, uint32_t vertex2)
:	m_material(material)
,	m_normal(c_InvalidIndex)
,	m_smoothGroup(0)
,	m_vertices(2)
{
	m_vertices[0] = vertex1;
	m_vertices[1] = vertex2;
}

Polygon::Polygon(uint32_t material, uint32_t vertex1, uint32_t vertex2, uint32_t vertex3)
:	m_material(material)
,	m_normal(c_InvalidIndex)
,	m_smoothGroup(0)
,	m_vertices(3)
{
	m_vertices[0] = vertex1;
	m_vertices[1] = vertex2;
	m_vertices[2] = vertex3;
}

Polygon::Polygon(uint32_t material, uint32_t vertex1, uint32_t vertex2, uint32_t vertex3, uint32_t vertex4)
:	m_material(material)
,	m_normal(c_InvalidIndex)
,	m_smoothGroup(0)
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

uint32_t Polygon::getMaterial() const
{
	return m_material;
}

void Polygon::setNormal(uint32_t normal)
{
	m_normal = normal;
}

uint32_t Polygon::getNormal() const
{
	return m_normal;
}

void Polygon::setSmoothGroup(uint32_t smoothGroup)
{
	m_smoothGroup = smoothGroup;
}

uint32_t Polygon::getSmoothGroup() const
{
	return m_smoothGroup;
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

void Polygon::setVertex(uint32_t index, uint32_t vertex)
{
	T_ASSERT (index < uint32_t(m_vertices.size()));
	m_vertices[index] = vertex;
}

uint32_t Polygon::getVertex(uint32_t index) const
{
	return m_vertices[index];
}

uint32_t Polygon::getVertexCount() const
{
	return uint32_t(m_vertices.size());
}

void Polygon::setVertices(const std::vector< uint32_t >& vertices)
{
	m_vertices = vertices;
}

const std::vector< uint32_t >& Polygon::getVertices() const
{
	return m_vertices;
}

bool Polygon::operator == (const Polygon& r) const
{
	return m_material == r.m_material && m_vertices == r.m_vertices;
}

	}
}
