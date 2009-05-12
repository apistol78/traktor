#include <algorithm>
#include "Model/Polygon.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.Polygon", Polygon, Object)

Polygon::Polygon()
:	m_material(c_InvalidIndex)
{
}

void Polygon::setMaterial(uint32_t material)
{
	m_material = material;
}

uint32_t Polygon::getMaterial() const
{
	return m_material;
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
