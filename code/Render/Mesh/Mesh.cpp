/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

void Mesh::setVertexBuffer(VertexBuffer* vertexBuffer)
{
	m_vertexBuffer = vertexBuffer;
}

void Mesh::setIndexBuffer(IndexBuffer* indexBuffer)
{
	m_indexBuffer = indexBuffer;
}

void Mesh::setParts(const std::vector< Part >& parts)
{
	m_parts = parts;
}

void Mesh::setBoundingBox(const Aabb3& boundingBox)
{
	m_boundingBox = boundingBox;
}

	}
}
