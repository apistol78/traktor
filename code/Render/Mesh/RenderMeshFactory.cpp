/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Mesh/RenderMeshFactory.h"
#include "Render/Mesh/Mesh.h"
#include "Render/IRenderSystem.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderMeshFactory", RenderMeshFactory, MeshFactory)

RenderMeshFactory::RenderMeshFactory(IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

Ref< Mesh > RenderMeshFactory::createMesh(
	const AlignedVector< VertexElement >& vertexElements,
	uint32_t vertexBufferSize,
	IndexType indexType,
	uint32_t indexBufferSize
) const
{
	Ref< const IVertexLayout > vertexLayout;
	Ref< Buffer > vertexBuffer;
	Ref< Buffer > indexBuffer;

	if (vertexBufferSize > 0)
	{
		const uint32_t vertexSize = getVertexSize(vertexElements);
		if (vertexSize == 0)
			return nullptr;

		vertexLayout = m_renderSystem->createVertexLayout(vertexElements);
		if (!vertexLayout)
			return nullptr;

		vertexBuffer = m_renderSystem->createBuffer(BuVertex, vertexBufferSize / vertexSize, vertexSize, false);
		if (!vertexBuffer)
			return nullptr;
	}

	if (indexBufferSize > 0)
	{
		const uint32_t indexSize = (indexType == IndexType::UInt16) ? 2 : 4;
		indexBuffer = m_renderSystem->createBuffer(BuIndex, indexBufferSize / indexSize, indexSize, false);
		if (!indexBuffer)
			return nullptr;
	}

	Ref< Mesh > mesh = new Mesh();
	mesh->setVertexElements(vertexElements);
	mesh->setVertexLayout(vertexLayout);
	mesh->setVertexBuffer(vertexBuffer);
	mesh->setIndexType(indexType);
	mesh->setIndexBuffer(indexBuffer);
	return mesh;
}

}
