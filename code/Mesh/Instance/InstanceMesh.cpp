#include "Mesh/Instance/InstanceMesh.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Context/RenderContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

render::handle_t s_handleInstanceWorld = 0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMesh", InstanceMesh, Object)

InstanceMesh::InstanceMesh()
{
	if (!s_handleInstanceWorld)
		s_handleInstanceWorld = render::getParameterHandle(L"InstanceWorld");
}

const Aabb& InstanceMesh::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void InstanceMesh::render(render::RenderContext* renderContext, const world::WorldRenderView* worldRenderView, const AlignedVector< InstanceMeshData >& instanceWorld)
{
	InstanceMeshData batchInstanceWorld[MaxInstanceCount];

	if (instanceWorld.empty())
		return;

	const std::vector< render::Mesh::Part >& parts = m_mesh->getParts();
	T_ASSERT (parts.size() == m_parts.size());

	for (size_t i = 0; i < parts.size(); ++i)
	{
		if (!m_parts[i].material.validate())
			continue;

		for (uint32_t batchOffset = 0; batchOffset < instanceWorld.size(); )
		{
			uint32_t batchCount = std::min< uint32_t >(uint32_t(instanceWorld.size()) - batchOffset, MaxInstanceCount);

			render::IndexedRenderBlock* renderBlock = renderContext->alloc< render::IndexedRenderBlock >();

			renderBlock->distance = 1e5f;
			renderBlock->shader = m_parts[i].material;
			renderBlock->shaderParams = renderContext->alloc< render::ShaderParameters >();
			renderBlock->indexBuffer = m_mesh->getIndexBuffer();
			renderBlock->vertexBuffer = m_mesh->getVertexBuffer();
			renderBlock->primitive = parts[i].primitives.type;
			renderBlock->offset = parts[i].primitives.offset;
			renderBlock->count = parts[i].primitives.count * batchCount;
			renderBlock->minIndex = parts[i].primitives.minIndex;
			renderBlock->maxIndex = parts[i].primitives.maxIndex;

			renderBlock->shaderParams->beginParameters(renderContext);
			renderBlock->shaderParams->setVectorArrayParameter(
				s_handleInstanceWorld,
				reinterpret_cast< const Vector4* >(&instanceWorld[batchOffset]),
				batchCount * sizeof(InstanceMeshData) / sizeof(Vector4)
			);
			worldRenderView->setShaderParameters(renderBlock->shaderParams);
			renderBlock->shaderParams->endParameters(renderContext);

			renderBlock->type = m_parts[i].material->isOpaque() ? render::RbtOpaque : render::RbtAlphaBlend;

			renderContext->draw(renderBlock);

			batchOffset += batchCount;
		}
	}
}

	}
}
