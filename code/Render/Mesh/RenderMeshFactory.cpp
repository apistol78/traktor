/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Mesh/RenderMeshFactory.h"
#include "Render/Mesh/Mesh.h"
#include "Render/IRenderSystem.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderMeshFactory", RenderMeshFactory, MeshFactory)

RenderMeshFactory::RenderMeshFactory(IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

Ref< Mesh > RenderMeshFactory::createMesh(
	const std::vector< VertexElement >& vertexElements,
	unsigned int vertexBufferSize,
	IndexType indexType,
	unsigned int indexBufferSize
)
{
	Ref< VertexBuffer > vertexBuffer;
	Ref< IndexBuffer > indexBuffer;

	if (vertexBufferSize > 0)
	{
		vertexBuffer = m_renderSystem->createVertexBuffer(vertexElements, vertexBufferSize, false);
		if (!vertexBuffer)
			return 0;
	}

	if (indexBufferSize > 0)
	{
		indexBuffer = m_renderSystem->createIndexBuffer(indexType, indexBufferSize, false);
		if (!indexBuffer)
			return 0;
	}

	Ref< Mesh > mesh = new Mesh();

	mesh->setVertexElements(vertexElements);
	mesh->setVertexBuffer(vertexBuffer);
	mesh->setIndexBuffer(indexBuffer);

	return mesh;
}

	}
}
