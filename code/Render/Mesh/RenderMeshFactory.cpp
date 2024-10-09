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
	uint32_t indexBufferSize,
	uint32_t auxBufferSize
) const
{
	Ref< const IVertexLayout > vertexLayout;
	Ref< Buffer > vertexBuffer;
	Ref< Buffer > indexBuffer;
	Ref< Buffer > auxBuffer;

	if (vertexBufferSize > 0)
	{
		const uint32_t vertexSize = getVertexSize(vertexElements);
		if (vertexSize == 0)
			return nullptr;

		vertexLayout = m_renderSystem->createVertexLayout(vertexElements);
		if (!vertexLayout)
			return nullptr;

		vertexBuffer = m_renderSystem->createBuffer(BuVertex, vertexBufferSize, false);
		if (!vertexBuffer)
			return nullptr;
	}

	if (indexBufferSize > 0)
	{
		indexBuffer = m_renderSystem->createBuffer(BuIndex, indexBufferSize, false);
		if (!indexBuffer)
			return nullptr;
	}

	if (auxBufferSize > 0)
	{
		auxBuffer = m_renderSystem->createBuffer(BuStructured, auxBufferSize, false);
		if (!auxBuffer)
			return nullptr;
	}

	Ref< Mesh > mesh = new Mesh();
	mesh->setVertexElements(vertexElements);
	mesh->setVertexLayout(vertexLayout);
	mesh->setVertexBuffer(vertexBuffer);
	mesh->setIndexType(indexType);
	mesh->setIndexBuffer(indexBuffer);
	mesh->setAuxBuffer(auxBuffer);
	return mesh;
}

}
