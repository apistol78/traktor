#include "Render/Mesh/Mesh.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Mesh", Mesh, Object)

void Mesh::setVertexElements(const std::vector< VertexElement >& vertexElements)
{
	m_vertexElements = vertexElements;
}

const std::vector< VertexElement >& Mesh::getVertexElements() const
{
	return m_vertexElements;
}

void Mesh::setVertexBuffer(VertexBuffer* vertexBuffer)
{
	m_vertexBuffer = vertexBuffer;
}

VertexBuffer* Mesh::getVertexBuffer() const
{
	return m_vertexBuffer;
}

void Mesh::setIndexBuffer(IndexBuffer* indexBuffer)
{
	m_indexBuffer = indexBuffer;
}

IndexBuffer* Mesh::getIndexBuffer() const
{
	return m_indexBuffer;
}

void Mesh::setParts(const std::vector< Part >& parts)
{
	m_parts = parts;
}

const std::vector< Mesh::Part >& Mesh::getParts() const
{
	return m_parts;
}

void Mesh::setBoundingBox(const Aabb& boundingBox)
{
	m_boundingBox = boundingBox;
}

const Aabb& Mesh::getBoundingBox() const
{
	return m_boundingBox;
}

	}
}
