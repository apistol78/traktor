/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Thread/JobManager.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Blend/BlendMesh.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/IWorldRenderPass.h"

#if !TARGET_OS_IPHONE
#	define T_USE_UPDATE_JOBS
#endif

namespace traktor::mesh
{
	namespace
	{

#if defined(T_USE_UPDATE_JOBS)
const uint32_t c_vertexCountFork = 1000;	//< Fork blend update on multiple threads if vertex count exceed this value.
#endif

struct BlendMeshTask
{
	const AlignedVector< render::VertexElement >& m_vertexElements;
	const AlignedVector< float >& m_blendWeights;
	const AlignedVector< const uint8_t* >& m_sourceVertexPtrs;
	uint8_t* m_destinationVertexPtr;
	uint32_t m_start;
	uint32_t m_end;

	BlendMeshTask(
		const AlignedVector< render::VertexElement >& vertexElements,
		const AlignedVector< float >& blendWeights,
		const AlignedVector< const uint8_t* >& sourceVertexPtrs,
		uint8_t* destinationVertexPtr,
		uint32_t start,
		uint32_t end
	)
	:	m_vertexElements(vertexElements)
	,	m_blendWeights(blendWeights)
	,	m_sourceVertexPtrs(sourceVertexPtrs)
	,	m_destinationVertexPtr(destinationVertexPtr)
	,	m_start(start)
	,	m_end(end)
	{
	}

	void execute()
	{
		uint32_t vertexSize = render::getVertexSize(m_vertexElements);
		uint32_t vertexOffset = m_start * vertexSize;

		uint8_t* destVerticesTop = m_destinationVertexPtr + vertexOffset;
		const uint8_t* baseVerticesTop = m_sourceVertexPtrs[0] + vertexOffset;

		std::memcpy(destVerticesTop, baseVerticesTop, (m_end - m_start) * vertexSize);

		for (uint32_t i = 0; i < m_blendWeights.size(); ++i)
		{
			float weight = m_blendWeights[i];

			if (std::abs(weight) <= FUZZY_EPSILON)
				continue;

			uint8_t* destVertices = destVerticesTop;
			const uint8_t* targetVertices = m_sourceVertexPtrs[i + 1] + vertexOffset;

			for (uint32_t j = m_start; j < m_end; ++j)
			{
				for (AlignedVector< render::VertexElement >::const_iterator k = m_vertexElements.begin(); k != m_vertexElements.end(); ++k)
				{
					uint8_t* destVertexElement = destVertices + k->getOffset();
					const uint8_t* targetVertexElement = targetVertices + k->getOffset();

					switch (k->getDataType())
					{
					case render::DtFloat1:
						reinterpret_cast< float* >(destVertexElement)[0] += reinterpret_cast< const float* >(targetVertexElement)[0] * weight;
						break;

					case render::DtFloat2:
						reinterpret_cast< float* >(destVertexElement)[0] += reinterpret_cast< const float* >(targetVertexElement)[0] * weight;
						reinterpret_cast< float* >(destVertexElement)[1] += reinterpret_cast< const float* >(targetVertexElement)[1] * weight;
						break;

					case render::DtFloat3:
						reinterpret_cast< float* >(destVertexElement)[0] += reinterpret_cast< const float* >(targetVertexElement)[0] * weight;
						reinterpret_cast< float* >(destVertexElement)[1] += reinterpret_cast< const float* >(targetVertexElement)[1] * weight;
						reinterpret_cast< float* >(destVertexElement)[2] += reinterpret_cast< const float* >(targetVertexElement)[2] * weight;
						break;

					case render::DtFloat4:
						reinterpret_cast< float* >(destVertexElement)[0] += reinterpret_cast< const float* >(targetVertexElement)[0] * weight;
						reinterpret_cast< float* >(destVertexElement)[1] += reinterpret_cast< const float* >(targetVertexElement)[1] * weight;
						reinterpret_cast< float* >(destVertexElement)[2] += reinterpret_cast< const float* >(targetVertexElement)[2] * weight;
						reinterpret_cast< float* >(destVertexElement)[3] += reinterpret_cast< const float* >(targetVertexElement)[3] * weight;
						break;

					default:
						break;
					}
				}
				destVertices += vertexSize;
				targetVertices += vertexSize;
			}
		}
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.BlendMesh", BlendMesh, IMesh)

const Aabb3& BlendMesh::getBoundingBox() const
{
	T_ASSERT(!m_meshes.empty());
	return m_meshes[0]->getBoundingBox();
}

bool BlendMesh::supportTechnique(render::handle_t technique) const
{
	return m_parts.find(technique) != m_parts.end();
}

uint32_t BlendMesh::getBlendTargetCount() const
{
	return uint32_t(m_meshes.size() - 1);
}

Ref< BlendMesh::Instance > BlendMesh::createInstance() const
{
	T_ASSERT(m_meshes[0]->getIndexBuffer());

	Ref< BlendMesh::Instance > instance = new BlendMesh::Instance();

	for (uint32_t i = 0; i < VertexBufferCount; ++i)
	{
		instance->vertexBuffers[i] = m_renderSystem->createBuffer(
			render::BuVertex,
			m_meshes[0]->getVertexBuffer()->getElementCount(),
			m_meshes[0]->getVertexBuffer()->getElementSize(),
			true
		);
		if (!instance->vertexBuffers[i])
			return nullptr;
	}

	Ref< render::Buffer > indexBuffer = m_renderSystem->createBuffer(
		render::BuIndex,
		m_meshes[0]->getIndexBuffer()->getElementCount(),
		m_meshes[0]->getIndexBuffer()->getElementSize(),
		false
	);
	if (!indexBuffer)
		return nullptr;

	// Copy indices from template's index buffer.
	void* sourceIndices = m_meshes[0]->getIndexBuffer()->lock();
	void* destIndices = indexBuffer->lock();
	if (!sourceIndices || !destIndices)
		return nullptr;
	std::memcpy(destIndices, sourceIndices, m_meshes[0]->getIndexBuffer()->getBufferSize());
	m_meshes[0]->getIndexBuffer()->unlock();
	indexBuffer->unlock();

	// Create render mesh.
	instance->mesh = new render::Mesh();
	instance->mesh->setVertexLayout(m_meshes[0]->getVertexLayout());
	instance->mesh->setVertexBuffer(0);
	instance->mesh->setIndexBuffer(indexBuffer);
	instance->mesh->setParts(m_meshes[0]->getParts());
	instance->mesh->setBoundingBox(m_meshes[0]->getBoundingBox());

	return instance;
}

void BlendMesh::build(
	render::RenderContext* renderContext,
	const world::IWorldRenderPass& worldRenderPass,
	const Transform& lastWorldTransform,
	const Transform& worldTransform,
	Instance* instance,
	const AlignedVector< float >& blendWeights,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	// Build render-able mesh by weighting in each blend shape's vertices.
	T_ASSERT(blendWeights.size() == getBlendTargetCount());

	// Update target mesh if weights has changed.
	{
		bool update = true;
		if (
			instance->mesh->getVertexBuffer() != nullptr &&
			blendWeights.size() == instance->weights.size()
		)
		{
			T_ASSERT(instance->count > 0);

			update = false;
			for (uint32_t i = 0; i < blendWeights.size(); ++i)
			{
				if (abs(blendWeights[i] - instance->weights[i]) >= FUZZY_EPSILON)
				{
					update = true;
					break;
				}
			}
		}
		if (update)
		{
			render::Buffer* vertexBuffer = instance->vertexBuffers[instance->count % VertexBufferCount];

			const AlignedVector< render::VertexElement >& vertexElements = instance->mesh->getVertexElements();
			uint32_t vertexSize = render::getVertexSize(vertexElements);
			uint32_t vertexCount = vertexBuffer->getBufferSize() / vertexSize;

			// Execute multiple tasks to perform blending.
			uint8_t* destinationVertices = static_cast< uint8_t* >(vertexBuffer->lock());
			if (destinationVertices)
			{
#if defined(T_USE_UPDATE_JOBS)
				if (vertexCount > c_vertexCountFork)
				{
					uint32_t pivots[] =
					{
						0,
						vertexCount / 4,
						(vertexCount * 2) / 4,
						(vertexCount * 3) / 4,
						vertexCount
					};

					BlendMeshTask task1(vertexElements, blendWeights, m_vertices, destinationVertices, pivots[0], pivots[1]);
					BlendMeshTask task2(vertexElements, blendWeights, m_vertices, destinationVertices, pivots[1], pivots[2]);
					BlendMeshTask task3(vertexElements, blendWeights, m_vertices, destinationVertices, pivots[2], pivots[3]);
					BlendMeshTask task4(vertexElements, blendWeights, m_vertices, destinationVertices, pivots[3], pivots[4]);

					Job::task_t jobs[4];
					jobs[0] = [&](){ task1.execute(); };
					jobs[1] = [&](){ task2.execute(); };
					jobs[2] = [&](){ task3.execute(); };
					jobs[3] = [&](){ task4.execute(); };
					JobManager::getInstance().fork(jobs, sizeof_array(jobs));
				}
				else
#endif
				{
					BlendMeshTask task(vertexElements, blendWeights, m_vertices, destinationVertices, 0, vertexCount);
					task.execute();
				}

				vertexBuffer->unlock();

				instance->mesh->setVertexBuffer(vertexBuffer);
				instance->weights = blendWeights;
				instance->count++;
			}
		}
	}

	if (!instance->mesh->getVertexBuffer())
		return;

	// Render mesh.
	SmallMap< render::handle_t, AlignedVector< Part > >::const_iterator it = m_parts.find(worldRenderPass.getTechnique());
	T_ASSERT(it != m_parts.end());

	const AlignedVector< render::Mesh::Part >& meshParts = instance->mesh->getParts();
	for (const auto& part : it->second)
	{
		auto permutation = worldRenderPass.getPermutation(m_shader);
		permutation.technique = part.shaderTechnique;
		auto sp = m_shader->getProgram(permutation);
		if (!sp)
			continue;

		auto renderBlock = renderContext->alloc< render::SimpleRenderBlock >(L"BlendMesh");
		renderBlock->distance = distance;
		renderBlock->program = sp.program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = instance->mesh->getIndexBuffer()->getBufferView();
		renderBlock->indexType = instance->mesh->getIndexType();
		renderBlock->vertexBuffer = instance->mesh->getVertexBuffer()->getBufferView();
		renderBlock->vertexLayout = instance->mesh->getVertexLayout();
		renderBlock->primitives = meshParts[part.meshPart].primitives;

		renderBlock->programParams->beginParameters(renderContext);
		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			lastWorldTransform,
			worldTransform
		);
		if (parameterCallback)
			parameterCallback->setParameters(renderBlock->programParams);
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(
			sp.priority,
			renderBlock
		);
	}
}

const std::map< std::wstring, int >& BlendMesh::getBlendTargetMap() const
{
	return m_targetMap;
}

}
