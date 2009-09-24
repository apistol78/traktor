#include <cstring>
#include "Mesh/Blend/BlendMesh.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Render/IRenderSystem.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/Shader.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Context/RenderContext.h"
#include "World/WorldRenderView.h"
#include "World/WorldRenderer.h"
#include "Core/Thread/JobManager.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

render::handle_t s_handleUserParameter = 0;

struct BlendMeshTask
{
	const std::vector< render::VertexElement >& m_vertexElements;
	const std::vector< float >& m_blendWeights;
	const std::vector< const uint8_t* >& m_sourceVertexPtrs;
	uint8_t* m_destinationVertexPtr;
	uint32_t m_start;
	uint32_t m_end;

	BlendMeshTask(
		const std::vector< render::VertexElement >& vertexElements,
		const std::vector< float >& blendWeights,
		const std::vector< const uint8_t* >& sourceVertexPtrs,
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
				for (std::vector< render::VertexElement >::const_iterator k = m_vertexElements.begin(); k != m_vertexElements.end(); ++k)
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
					}
				}
				destVertices += vertexSize;
				targetVertices += vertexSize;
			}
		}
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.BlendMesh", BlendMesh, Object)

BlendMesh::BlendMesh()
{
	if (!s_handleUserParameter)
		s_handleUserParameter = render::getParameterHandle(L"UserParameter");
}

const Aabb& BlendMesh::getBoundingBox() const
{
	T_ASSERT (!m_meshes.empty());
	return m_meshes[0]->getBoundingBox();
}

uint32_t BlendMesh::getBlendTargetCount() const
{
	return uint32_t(m_meshes.size() - 1);
}

BlendMesh::Instance* BlendMesh::createInstance() const
{
	T_ASSERT (m_meshes[0]->getIndexBuffer());

	Ref< render::VertexBuffer > vertexBuffer = m_renderSystem->createVertexBuffer(
		m_meshes[0]->getVertexElements(),
		m_meshes[0]->getVertexBuffer()->getBufferSize(),
		true
	);
	if (!vertexBuffer)
		return 0;

	Ref< render::IndexBuffer > indexBuffer = m_renderSystem->createIndexBuffer(
		m_meshes[0]->getIndexBuffer()->getIndexType(),
		m_meshes[0]->getIndexBuffer()->getBufferSize(),
		false
	);
	if (!indexBuffer)
		return 0;

	// Copy indices from template's index buffer.
	void* sourceIndices = m_meshes[0]->getIndexBuffer()->lock();
	void* destIndices = indexBuffer->lock();
	if (!sourceIndices || !destIndices)
		return 0;
	std::memcpy(destIndices, sourceIndices, m_meshes[0]->getIndexBuffer()->getBufferSize());
	m_meshes[0]->getIndexBuffer()->unlock();
	indexBuffer->unlock();

	// Create instance.
	Ref< BlendMesh::Instance > instance = gc_new< BlendMesh::Instance >();

	instance->mesh = gc_new< render::Mesh >();
	instance->mesh->setVertexElements(m_meshes[0]->getVertexElements());
	instance->mesh->setVertexBuffer(vertexBuffer);
	instance->mesh->setIndexBuffer(indexBuffer);
	instance->mesh->setParts(m_meshes[0]->getParts());
	instance->mesh->setBoundingBox(m_meshes[0]->getBoundingBox());

	return instance;
}

void BlendMesh::render(
	render::RenderContext* renderContext,
	const world::WorldRenderView* worldRenderView,
	const Transform& worldTransform,
	Instance* instance,
	const std::vector< float >& blendWeights,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	// Build renderable mesh by weighting in each blend shape's vertices.
	T_ASSERT (blendWeights.size() == getBlendTargetCount());

	render::handle_t technique = worldRenderView->getTechnique();

	// Update target mesh only when we're rendering default technique.
	if (technique == world::WorldRenderer::getTechniqueDefault())
	{
		const std::vector< render::VertexElement >& vertexElements = instance->mesh->getVertexElements();
		uint32_t vertexSize = render::getVertexSize(vertexElements);
		uint32_t vertexCount = instance->mesh->getVertexBuffer()->getBufferSize() / vertexSize;

		// Execute multiple tasks to perform blending.
		uint8_t* destinationVertices = static_cast< uint8_t* >(instance->mesh->getVertexBuffer()->lock());

#if 1
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

		Job jobs[] =
		{
			makeFunctor< BlendMeshTask >(&task1, &BlendMeshTask::execute),
			makeFunctor< BlendMeshTask >(&task2, &BlendMeshTask::execute),
			makeFunctor< BlendMeshTask >(&task3, &BlendMeshTask::execute),
			makeFunctor< BlendMeshTask >(&task4, &BlendMeshTask::execute)
		};

		JobManager::getInstance().fork(jobs, sizeof_array(jobs));
#else
		BlendMeshTask task(vertexElements, blendWeights, m_vertices, destinationVertices, 0, vertexCount);
		task.execute();
#endif

		instance->mesh->getVertexBuffer()->unlock();
	}

	// Render mesh.
	const std::vector< render::Mesh::Part >& parts = instance->mesh->getParts();
	T_ASSERT (parts.size() == m_parts.size());

	for (size_t i = 0; i < parts.size(); ++i)
	{
		if (!m_parts[i].material.validate())
			continue;

		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >();

		renderBlock->distance = distance;
		renderBlock->shader = m_parts[i].material;
		renderBlock->shaderParams = renderContext->alloc< render::ShaderParameters >();
		renderBlock->indexBuffer = instance->mesh->getIndexBuffer();
		renderBlock->vertexBuffer = instance->mesh->getVertexBuffer();
		renderBlock->primitives = &parts[i].primitives;

		renderBlock->shaderParams->beginParameters(renderContext);
		if (parameterCallback)
			parameterCallback->setParameters(renderBlock->shaderParams);
		worldRenderView->setShaderParameters(
			renderBlock->shaderParams,
			worldTransform.toMatrix44(),
			worldTransform.toMatrix44(),	// @fixme
			getBoundingBox()
		);
		renderBlock->shaderParams->endParameters(renderContext);

		renderBlock->type = m_parts[i].material->isOpaque() ? render::RbtOpaque : render::RbtAlphaBlend;

		renderContext->draw(renderBlock);
	}
}

const std::map< std::wstring, int >& BlendMesh::getBlendTargetMap() const
{
	return m_targetMap;
}

	}
}
