#include "Render/Mesh/Mesh.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Mesh", Mesh, Object)

void Mesh::setVertexElements(const AlignedVector< VertexElement >& vertexElements)
{
	m_vertexElements = vertexElements;
}

void Mesh::setVertexLayout(const IVertexLayout* vertexLayout)
{
	m_vertexLayout = vertexLayout;
}

void Mesh::setVertexBuffer(Buffer* vertexBuffer)
{
	m_vertexBuffer = vertexBuffer;
}

void Mesh::setIndexType(IndexType indexType)
{
	m_indexType = indexType;
}

void Mesh::setIndexBuffer(Buffer* indexBuffer)
{
	m_indexBuffer = indexBuffer;
}

void Mesh::setParts(const AlignedVector< Part >& parts)
{
	m_parts = parts;
}

void Mesh::setBoundingBox(const Aabb3& boundingBox)
{
	m_boundingBox = boundingBox;
}

	}
}
