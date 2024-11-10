/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Mesh/Mesh.h"

namespace traktor::render
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

void Mesh::setAuxBuffer(const FourCC& id, Buffer* auxBuffer)
{
	m_auxBuffers[id] = auxBuffer;
}

void Mesh::setAuxBuffers(const SmallMap< FourCC, Ref< Buffer > >& auxBuffers)
{
	m_auxBuffers = auxBuffers;
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
