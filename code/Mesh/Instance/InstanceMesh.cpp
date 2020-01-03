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
render::handle_t s_handleInstanceWorldLast = 0;

struct SortRenderInstance
{
	bool operator () (const InstanceMesh::RenderInstance& d1, const InstanceMesh::RenderInstance& d2) const
	{
		return d1.distance < d2.distance;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMesh", InstanceMesh, IMesh)

InstanceMesh::InstanceMesh()
:	m_maxInstanceCount(0)
{
	if (!s_handleInstanceWorld)
		s_handleInstanceWorld = render::getParameterHandle(L"InstanceWorld");
	if (!s_handleInstanceWorldLast)
		s_handleInstanceWorldLast = render::getParameterHandle(L"InstanceWorldLast");
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

void InstanceMesh::getTechniques(SmallSet< render::handle_t >& outHandles) const
{
	for (const auto part : m_parts)
		outHandles.insert(part.first);
}

void InstanceMesh::render(
	render::RenderContext* renderContext,
	const world::IWorldRenderPass& worldRenderPass,
	AlignedVector< RenderInstance >& instanceWorld,
	render::ProgramParameters* extraParameters
)
{
	InstanceMeshData T_ALIGN16 instanceBatch[MaxInstanceCount];
	InstanceMeshData T_ALIGN16 instanceLastBatch[MaxInstanceCount];
	bool haveAlphaBlend = false;

	if (instanceWorld.empty())
		return;

	auto it = m_parts.find(worldRenderPass.getTechnique());
	if (it == m_parts.end())
		return;

	// Sort instances by ascending distance; note we're sorting caller's vector.
	std::sort(instanceWorld.begin(), instanceWorld.end(), SortRenderInstance());

	// Calculate bounding box of all instances.
	Aabb3 boundingBoxLocal = m_renderMesh->getBoundingBox();
	Aabb3 boundingBoxWorld;
	for (AlignedVector< RenderInstance >::const_iterator i = instanceWorld.begin(); i != instanceWorld.end(); ++i)
	{
		Vector4 translation(
			i->data.translation[0],
			i->data.translation[1],
			i->data.translation[2],
			0.0f
		);

		Quaternion rotation(
			i->data.rotation[0],
			i->data.rotation[1],
			i->data.rotation[2],
			i->data.rotation[3]
		);

		boundingBoxWorld.contain(boundingBoxLocal.transform(Transform(
			translation,
			rotation
		)));
	}

	const AlignedVector< render::Mesh::Part >& meshParts = m_renderMesh->getParts();

	// Render opaque parts front-to-back.
	for (const auto& part : it->second)
	{
		m_shader->setTechnique(part.shaderTechnique);
		worldRenderPass.setShaderCombination(m_shader);

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
		batchParameters->beginParameters(renderContext);
		batchParameters->attachParameters(extraParameters);
		worldRenderPass.setProgramParameters(
			batchParameters,
			Transform::identity(),
			Transform::identity(),
			boundingBoxWorld
		);
		batchParameters->endParameters(renderContext);

		for (uint32_t batchOffset = 0; batchOffset < instanceWorld.size(); )
		{
			uint32_t batchCount = std::min< uint32_t >(uint32_t(instanceWorld.size()) - batchOffset, m_maxInstanceCount);

			for (uint32_t j = 0; j < batchCount; ++j)
			{
				instanceBatch[j] = instanceWorld[batchOffset + j].data;
				instanceLastBatch[j] = instanceWorld[batchOffset + j].data0;
			}

			render::InstancingRenderBlock* renderBlock = renderContext->alloc< render::InstancingRenderBlock >("InstanceMesh opaque");

			renderBlock->distance = instanceWorld[batchOffset + batchCount - 1].distance;
			renderBlock->program = program;
			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->indexBuffer = m_renderMesh->getIndexBuffer();
			renderBlock->vertexBuffer = m_renderMesh->getVertexBuffer();
			renderBlock->primitives = meshParts[part.meshPart].primitives;
			renderBlock->count = batchCount;

			renderBlock->programParams->beginParameters(renderContext);
			renderBlock->programParams->attachParameters(batchParameters);
			renderBlock->programParams->setVectorArrayParameter(
				s_handleInstanceWorld,
				reinterpret_cast< const Vector4* >(instanceBatch),
				batchCount * sizeof(InstanceMeshData) / sizeof(Vector4)
			);
			renderBlock->programParams->setVectorArrayParameter(
				s_handleInstanceWorldLast,
				reinterpret_cast< const Vector4* >(instanceLastBatch),
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

		for (const auto& part : it->second)
		{
			m_shader->setTechnique(part.shaderTechnique);
			worldRenderPass.setShaderCombination(m_shader);

			if ((m_shader->getCurrentPriority() & (render::RpAlphaBlend | render::RpPostAlphaBlend)) == 0)
				continue;

			render::IProgram* program = m_shader->getCurrentProgram();
			if (!program)
				continue;

			// Setup batch shared parameters.
			render::ProgramParameters* batchParameters = renderContext->alloc< render::ProgramParameters >();
			batchParameters->beginParameters(renderContext);
			batchParameters->attachParameters(extraParameters);
			worldRenderPass.setProgramParameters(
				batchParameters,
				Transform::identity(),
				Transform::identity(),
				boundingBoxWorld
			);
			batchParameters->endParameters(renderContext);

			for (uint32_t batchOffset = 0; batchOffset < instanceWorld.size(); )
			{
				uint32_t batchCount = std::min< uint32_t >(uint32_t(instanceWorld.size()) - batchOffset, m_maxInstanceCount);

				for (uint32_t j = 0; j < batchCount; ++j)
				{
					instanceBatch[j] = instanceWorld[batchOffset + j].data;
					instanceLastBatch[j] = instanceWorld[batchOffset + j].data0;
				}

				render::InstancingRenderBlock* renderBlock = renderContext->alloc< render::InstancingRenderBlock >("InstanceMesh blend");

				renderBlock->distance = instanceWorld[batchOffset + batchCount - 1].distance;
				renderBlock->program = program;
				renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlock->indexBuffer = m_renderMesh->getIndexBuffer();
				renderBlock->vertexBuffer = m_renderMesh->getVertexBuffer();
				renderBlock->primitives = meshParts[part.meshPart].primitives;
				renderBlock->count = batchCount;

				renderBlock->programParams->beginParameters(renderContext);
				renderBlock->programParams->attachParameters(batchParameters);
				renderBlock->programParams->setVectorArrayParameter(
					s_handleInstanceWorld,
					reinterpret_cast< const Vector4* >(instanceBatch),
					batchCount * sizeof(InstanceMeshData) / sizeof(Vector4)
				);
				renderBlock->programParams->setVectorArrayParameter(
					s_handleInstanceWorldLast,
					reinterpret_cast< const Vector4* >(instanceLastBatch),
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
