/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Log/Log.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Render/Buffer.h"
#include "Render/IProgram.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/IWorldRenderPass.h"

namespace traktor::mesh
{
	namespace
	{

render::Handle s_handleInstanceWorld(L"InstanceWorld");
render::Handle s_handleInstanceWorldLast(L"InstanceWorldLast");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMesh", InstanceMesh, IMesh)

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

void InstanceMesh::build(
	render::RenderContext* renderContext,
	const world::IWorldRenderPass& worldRenderPass,
	AlignedVector< RenderInstance >& instanceWorld,
	render::ProgramParameters* extraParameters
) const
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
	std::sort(instanceWorld.begin(), instanceWorld.end(), [](const InstanceMesh::RenderInstance& d1, const InstanceMesh::RenderInstance& d2) {
		return d1.distance < d2.distance;
	});

	const auto& meshParts = m_renderMesh->getParts();

	// Render opaque parts front-to-back.
	for (const auto& part : it->second)
	{
		auto permutation = worldRenderPass.getPermutation(m_shader);
		permutation.technique = part.shaderTechnique;
		auto sp = m_shader->getProgram(permutation);
		if (!sp)
			continue;

		if ((sp.priority & (render::RpAlphaBlend | render::RpPostAlphaBlend)) != 0)
		{
			haveAlphaBlend = true;
			continue;
		}

		// Setup batch shared parameters.
		render::ProgramParameters* batchParameters = renderContext->alloc< render::ProgramParameters >();
		batchParameters->beginParameters(renderContext);

		if (extraParameters)
			batchParameters->attachParameters(extraParameters);

		worldRenderPass.setProgramParameters(
			batchParameters,
			Transform::identity(),
			Transform::identity()
		);
		batchParameters->endParameters(renderContext);

		for (uint32_t batchOffset = 0; batchOffset < instanceWorld.size(); )
		{
			const uint32_t batchCount = std::min< uint32_t >(uint32_t(instanceWorld.size()) - batchOffset, m_maxInstanceCount);

			for (uint32_t j = 0; j < batchCount; ++j)
			{
				instanceBatch[j] = instanceWorld[batchOffset + j].data;
				instanceLastBatch[j] = instanceWorld[batchOffset + j].data0;
			}

			auto renderBlock = renderContext->alloc< render::InstancingRenderBlock >(L"InstanceMesh opaque");
			renderBlock->distance = instanceWorld[batchOffset + batchCount - 1].distance;
			renderBlock->program = sp.program;
			renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
			renderBlock->indexBuffer = m_renderMesh->getIndexBuffer()->getBufferView();
			renderBlock->indexType = m_renderMesh->getIndexType();
			renderBlock->vertexBuffer = m_renderMesh->getVertexBuffer()->getBufferView();
			renderBlock->vertexLayout = m_renderMesh->getVertexLayout();
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

			renderContext->draw(sp.priority, renderBlock);

			batchOffset += batchCount;
		}
	}

	// Render alpha blend parts back-to-front.
	if (haveAlphaBlend)
	{
		std::reverse(instanceWorld.begin(), instanceWorld.end());

		for (const auto& part : it->second)
		{
			auto permutation = worldRenderPass.getPermutation(m_shader);
			permutation.technique = part.shaderTechnique;
			auto sp = m_shader->getProgram(permutation);
			if (!sp)
				continue;

			if ((sp.priority & (render::RpAlphaBlend | render::RpPostAlphaBlend)) == 0)
				continue;

			// Setup batch shared parameters.
			render::ProgramParameters* batchParameters = renderContext->alloc< render::ProgramParameters >();
			batchParameters->beginParameters(renderContext);

			if (extraParameters)
				batchParameters->attachParameters(extraParameters);

			worldRenderPass.setProgramParameters(
				batchParameters,
				Transform::identity(),
				Transform::identity()
			);
			batchParameters->endParameters(renderContext);

			for (uint32_t batchOffset = 0; batchOffset < instanceWorld.size(); )
			{
				const uint32_t batchCount = std::min< uint32_t >(uint32_t(instanceWorld.size()) - batchOffset, m_maxInstanceCount);

				for (uint32_t j = 0; j < batchCount; ++j)
				{
					instanceBatch[j] = instanceWorld[batchOffset + j].data;
					instanceLastBatch[j] = instanceWorld[batchOffset + j].data0;
				}

				auto renderBlock = renderContext->alloc< render::InstancingRenderBlock >(L"InstanceMesh blend");
				renderBlock->distance = instanceWorld[batchOffset + batchCount - 1].distance;
				renderBlock->program = sp.program;
				renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlock->indexBuffer = m_renderMesh->getIndexBuffer()->getBufferView();
				renderBlock->indexType = m_renderMesh->getIndexType();
				renderBlock->vertexBuffer = m_renderMesh->getVertexBuffer()->getBufferView();
				renderBlock->vertexLayout = m_renderMesh->getVertexLayout();
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

				renderContext->draw(sp.priority, renderBlock);

				batchOffset += batchCount;
			}
		}
	}
}

}
