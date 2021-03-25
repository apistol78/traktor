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
	const AlignedVector< VertexElement >& vertexElements,
	uint32_t vertexBufferSize,
	IndexType indexType,
	uint32_t indexBufferSize
) const
{
	Ref< VertexBuffer > vertexBuffer;
	Ref< IndexBuffer > indexBuffer;

	if (vertexBufferSize > 0)
	{
		vertexBuffer = m_renderSystem->createVertexBuffer(vertexElements, vertexBufferSize, false);
		if (!vertexBuffer)
			return nullptr;
	}

	if (indexBufferSize > 0)
	{
		indexBuffer = m_renderSystem->createIndexBuffer(indexType, indexBufferSize, false);
		if (!indexBuffer)
			return nullptr;
	}

	Ref< Mesh > mesh = new Mesh();
	mesh->setVertexElements(vertexElements);
	mesh->setVertexBuffer(vertexBuffer);
	mesh->setIndexBuffer(indexBuffer);
	return mesh;
}

	}
}
