#include <algorithm>
#include "Core/Log/Log.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Render/IProgram.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/IWorldRenderPass.h"

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
:	m_maxInstanceCount(0)
{
	if (!s_handleInstanceWorld)
		s_handleInstanceWorld = render::getParameterHandle(L"InstanceWorld");
}

InstanceMesh::~InstanceMesh()
{
}

const Aabb3& InstanceMesh::getBoundingBox() const
{
	return m_renderMesh->getBoundingBox();
}

bool InstanceMesh::supportTechnique(render::handle_t technique) const
{
	return m_parts.find(technique) != m_parts.end();
}

void InstanceMesh::getTechniques(std::set< render::handle_t >& outHandles) const
{
	for (SmallMap< render::handle_t, std::vector< Part > >::const_iterator i = m_parts.begin(); i != m_parts.end(); ++i)
		outHandles.insert(i->first);
}

void InstanceMesh::render(
	render::RenderContext* renderContext,
	const world::IWorldRenderPass& worldRenderPass,
	AlignedVector< instance_distance_t >& instanceWorld,
	render::ProgramParameters* extraParameters
)
{
	InstanceMeshData T_ALIGN16 instanceBatch[MaxInstanceCount];
	bool haveAlphaBlend = false;

	if (instanceWorld.empty())
		return;

	SmallMap< render::handle_t, std::vector< Part > >::const_iterator it = m_parts.find(worldRenderPass.getTechnique());
	T_ASSERT (it != m_parts.end());

	// Sort instances by ascending distance; note we're sorting caller's vector.
	std::sort(instanceWorld.begin(), instanceWorld.end(), SortInstanceDistance());

	// Calculate bounding box of all instances.
	Aabb3 boundingBoxLocal = m_renderMesh->getBoundingBox();
	Aabb3 boundingBoxWorld;
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
	Transform boundingBoxCenter(boundingBoxWorld.getCenter());
	boundingBoxWorld.transform(Transform(-boundingBoxWorld.getCenter()));

	const std::vector< render::Mesh::Part >& meshParts = m_renderMesh->getParts();

	// Render opaque parts front-to-back.
	for (std::vector< Part >::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
	{
		m_shader->setTechnique(i->shaderTechnique);
		worldRenderPass.setShaderCombination(
			m_shader,
			boundingBoxCenter,
			boundingBoxWorld
		);

		if ((m_shader->getCurrentPriority() & (render::RpAlphaBlend | render::RpPostAlphaBlend)) != 0)
		{
			haveAlphaBlend = true;
			continue;
		}

		render::IProgram* program = m_shader->getCurrentProgram();
		if (!program)
			continue;

		// Setup batch shared parameters.
		render::ProgramParameters* batchParameters = renderContext->alloc< render::ProgramParameters >();
		batchParameters->attachParameters(extraParameters);
		batchParameters->beginParameters(renderContext);
		worldRenderPass.setProgramParameters(
			batchParameters,
			boundingBoxCenter,
			boundingBoxCenter,
			boundingBoxWorld
		);
		batchParameters->endParameters(renderContext);

		for (uint32_t batchOffset = 0; batchOffset < instanceWorld.size(); )
		{
			uint32_t batchCount = std::min< uint32_t >(uint32_t(instanceWorld.size()) - batchOffset, m_maxInstanceCount);

			for (uint32_t j = 0; j < batchCount; ++j)
				instanceBatch[j] = instanceWorld[batchOffset + j].first;

#if !T_USE_LEGACY_INSTANCING

			render::InstancingRenderBlock* renderBlock = renderContext->alloc< render::InstancingRenderBlock >("InstanceMesh opaque");

			renderBlock->distance = instanceWorld[batchOffset + batchCount - 1].second;
			renderBlock->program = program;
			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->indexBuffer = m_renderMesh->getIndexBuffer();
			renderBlock->vertexBuffer = m_renderMesh->getVertexBuffer();
			renderBlock->primitives = meshParts[i->meshPart].primitives;
			renderBlock->count = batchCount;

#else

			render::IndexedRenderBlock* renderBlock = renderContext->alloc< render::IndexedRenderBlock >("InstanceMesh opaque");

			renderBlock->distance = instanceWorld[batchOffset + batchCount - 1].second;
			renderBlock->program = program;
			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->indexBuffer = m_renderMesh->getIndexBuffer();
			renderBlock->vertexBuffer = m_renderMesh->getVertexBuffer();
			renderBlock->primitive = meshParts[i->meshPart].primitives.type;
			renderBlock->offset = meshParts[i->meshPart].primitives.offset;
			renderBlock->count = meshParts[i->meshPart].primitives.count * batchCount;
			renderBlock->minIndex = meshParts[i->meshPart].primitives.minIndex;
			renderBlock->maxIndex = meshParts[i->meshPart].primitives.maxIndex;

#endif

			renderBlock->programParams->attachParameters(batchParameters);
			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->setVectorArrayParameter(
				s_handleInstanceWorld,
				reinterpret_cast< const Vector4* >(instanceBatch),
				batchCount * sizeof(InstanceMeshData) / sizeof(Vector4)
			);
			renderBlock->programParams->endParameters(renderContext);

			renderContext->draw(m_shader->getCurrentPriority(), renderBlock);

			batchOffset += batchCount;
		}
	}

	// Render alpha blend parts back-to-front.
	if (haveAlphaBlend)
	{
		std::reverse(instanceWorld.begin(), instanceWorld.end());

		for (std::vector< Part >::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
		{
			m_shader->setTechnique(i->shaderTechnique);
			worldRenderPass.setShaderCombination(
				m_shader,
				boundingBoxCenter,
				boundingBoxWorld
			);

			if ((m_shader->getCurrentPriority() & (render::RpAlphaBlend | render::RpPostAlphaBlend)) == 0)
				continue;

			render::IProgram* program = m_shader->getCurrentProgram();
			if (!program)
				continue;

			// Setup batch shared parameters.
			render::ProgramParameters* batchParameters = renderContext->alloc< render::ProgramParameters >();
			batchParameters->attachParameters(extraParameters);
			batchParameters->beginParameters(renderContext);
			worldRenderPass.setProgramParameters(
				batchParameters,
				boundingBoxCenter,
				boundingBoxCenter,
				boundingBoxWorld
			);
			batchParameters->endParameters(renderContext);

			for (uint32_t batchOffset = 0; batchOffset < instanceWorld.size(); )
			{
				uint32_t batchCount = std::min< uint32_t >(uint32_t(instanceWorld.size()) - batchOffset, m_maxInstanceCount);

				for (uint32_t j = 0; j < batchCount; ++j)
					instanceBatch[j] = instanceWorld[batchOffset + j].first;

#if !T_USE_LEGACY_INSTANCING

				render::InstancingRenderBlock* renderBlock = renderContext->alloc< render::InstancingRenderBlock >("InstanceMesh blend");

				renderBlock->distance = instanceWorld[batchOffset + batchCount - 1].second;
				renderBlock->program = program;
				renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlock->indexBuffer = m_renderMesh->getIndexBuffer();
				renderBlock->vertexBuffer = m_renderMesh->getVertexBuffer();
				renderBlock->primitives = meshParts[i->meshPart].primitives;
				renderBlock->count = batchCount;

#else

				render::IndexedRenderBlock* renderBlock = renderContext->alloc< render::IndexedRenderBlock >("InstanceMesh blend");

				renderBlock->distance = instanceWorld[batchOffset + batchCount - 1].second;
				renderBlock->program = program;
				renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlock->indexBuffer = m_renderMesh->getIndexBuffer();
				renderBlock->vertexBuffer = m_renderMesh->getVertexBuffer();
				renderBlock->primitive = meshParts[i->meshPart].primitives.type;
				renderBlock->offset = meshParts[i->meshPart].primitives.offset;
				renderBlock->count = meshParts[i->meshPart].primitives.count * batchCount;
				renderBlock->minIndex = meshParts[i->meshPart].primitives.minIndex;
				renderBlock->maxIndex = meshParts[i->meshPart].primitives.maxIndex;

#endif

				renderBlock->programParams->attachParameters(batchParameters);
				renderBlock->programParams->beginParameters(renderContext);
				renderBlock->programParams->setVectorArrayParameter(
					s_handleInstanceWorld,
					reinterpret_cast< const Vector4* >(instanceBatch),
					batchCount * sizeof(InstanceMeshData) / sizeof(Vector4)
				);
				renderBlock->programParams->endParameters(renderContext);

				renderContext->draw(m_shader->getCurrentPriority(), renderBlock);

				batchOffset += batchCount;
			}
		}
	}
}

	}
}
