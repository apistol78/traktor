#include <algorithm>
#include "Mesh/Instance/InstanceMesh.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

render::handle_t s_handleInstanceWorld = 0;

struct SortInstanceDistance
{
	bool operator () (const InstanceMesh::instance_distance_t& d1, const InstanceMesh::instance_distance_t& d2) const
	{
		return d1.second < d2.second;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMesh", InstanceMesh, IMesh)

InstanceMesh::InstanceMesh()
{
	if (!s_handleInstanceWorld)
		s_handleInstanceWorld = render::getParameterHandle(L"InstanceWorld");
}

const Aabb& InstanceMesh::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void InstanceMesh::render(render::RenderContext* renderContext, const world::WorldRenderView* worldRenderView, AlignedVector< instance_distance_t >& instanceWorld)
{
	InstanceMeshData T_ALIGN16 instanceBatch[MaxInstanceCount];
	bool haveAlphaBlend = false;

	if (instanceWorld.empty())
		return;

	// Sort instances by ascending distance; note we're sorting caller's vector.
	std::sort(instanceWorld.begin(), instanceWorld.end(), SortInstanceDistance());

	// Calculate bounding box of all instances.
	Aabb boundingBoxLocal = m_mesh->getBoundingBox();
	Aabb boundingBoxWorld;
	for (AlignedVector< instance_distance_t >::const_iterator i = instanceWorld.begin(); i != instanceWorld.end(); ++i)
	{
		Vector4 translation(
			i->first.translation[0],
			i->first.translation[1],
			i->first.translation[2],
			0.0f
		);

		Quaternion rotation(
			i->first.rotation[0],
			i->first.rotation[1],
			i->first.rotation[2],
			i->first.rotation[3]
		);

		boundingBoxWorld.contain(boundingBoxLocal.transform(Transform(
			translation,
			rotation
		)));
	}

	// Transform bounding box into origo; keep translation as matrix.
	Matrix44 boundingBoxCenter = translate(boundingBoxWorld.getCenter());
	boundingBoxWorld.transform(Transform(-boundingBoxWorld.getCenter()));

	const std::vector< render::Mesh::Part >& parts = m_mesh->getParts();
	T_ASSERT (parts.size() == m_parts.size());

	// Render opaque parts front-to-back.
	for (size_t i = 0; i < parts.size(); ++i)
	{
		if (!m_parts[i].material.validate())
			continue;
		if (!m_parts[i].material->hasTechnique(worldRenderView->getTechnique()))
			continue;
		if (!m_parts[i].opaque)
		{
			haveAlphaBlend = true;
			continue;
		}

		for (uint32_t batchOffset = 0; batchOffset < instanceWorld.size(); )
		{
			uint32_t batchCount = std::min< uint32_t >(uint32_t(instanceWorld.size()) - batchOffset, MaxInstanceCount);

			for (uint32_t j = 0; j < batchCount; ++j)
				instanceBatch[j] = instanceWorld[batchOffset + j].first;

			render::IndexedRenderBlock* renderBlock = renderContext->alloc< render::IndexedRenderBlock >();

			renderBlock->distance = instanceWorld[batchOffset].second;
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
			worldRenderView->setShaderParameters(renderBlock->shaderParams, boundingBoxCenter, boundingBoxCenter, boundingBoxWorld);
			renderBlock->shaderParams->setVectorArrayParameter(
				s_handleInstanceWorld,
				reinterpret_cast< const Vector4* >(instanceBatch),
				batchCount * sizeof(InstanceMeshData) / sizeof(Vector4)
			);
			renderBlock->shaderParams->endParameters(renderContext);

			renderContext->draw(render::RfOpaque, renderBlock);

			batchOffset += batchCount;
		}
	}

	// Render alpha blend parts back-to-front.
	if (haveAlphaBlend)
	{
		std::reverse(instanceWorld.begin(), instanceWorld.end());

		for (size_t i = 0; i < parts.size(); ++i)
		{
			if (!m_parts[i].material.validate() || m_parts[i].opaque)
				continue;

			for (uint32_t batchOffset = 0; batchOffset < instanceWorld.size(); )
			{
				uint32_t batchCount = std::min< uint32_t >(uint32_t(instanceWorld.size()) - batchOffset, MaxInstanceCount);

				for (uint32_t j = 0; j < batchCount; ++j)
					instanceBatch[j] = instanceWorld[batchOffset + j].first;

				render::IndexedRenderBlock* renderBlock = renderContext->alloc< render::IndexedRenderBlock >();

				renderBlock->distance = instanceWorld[batchOffset].second;
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
				worldRenderView->setShaderParameters(renderBlock->shaderParams);
				renderBlock->shaderParams->setVectorArrayParameter(
					s_handleInstanceWorld,
					reinterpret_cast< const Vector4* >(instanceBatch),
					batchCount * sizeof(InstanceMeshData) / sizeof(Vector4)
				);
				renderBlock->shaderParams->endParameters(renderContext);

				renderContext->draw(render::RfAlphaBlend, renderBlock);

				batchOffset += batchCount;
			}
		}
	}
}

	}
}
