#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Static/StaticMesh.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/IWorldCulling.h"
#include "World/IWorldRenderPass.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StaticMesh", StaticMesh, IMesh)

const Aabb3& StaticMesh::getBoundingBox() const
{
	return m_renderMesh->getBoundingBox();
}

bool StaticMesh::supportTechnique(render::handle_t technique) const
{
	return m_parts.find(technique) != m_parts.end();
}

void StaticMesh::precull(
	world::IWorldCulling* worldCulling,
	const Transform& worldTransform
)
{
	if (m_occluderMesh)
		worldCulling->placeOccluder(m_occluderMesh, worldTransform);
}

void StaticMesh::render(
	render::RenderContext* renderContext,
	const world::IWorldRenderPass& worldRenderPass,
	const Transform& worldTransform,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	SmallMap< render::handle_t, std::vector< Part > >::const_iterator it = m_parts.find(worldRenderPass.getTechnique());
	T_ASSERT (it != m_parts.end());

	Matrix44 transform = worldTransform.toMatrix44();
	Aabb3 boundingBox = m_renderMesh->getBoundingBox();

	const std::vector< render::Mesh::Part >& meshParts = m_renderMesh->getParts();
	for (std::vector< Part >::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
	{
		m_shader->setTechnique(i->shaderTechnique);

		worldRenderPass.setShaderCombination(
			m_shader,
			transform,
			boundingBox
		);

		if (parameterCallback)
			parameterCallback->setCombination(m_shader);

		render::IProgram* program = m_shader->getCurrentProgram();
		if (!program)
			continue;

#if !defined(_DEBUG)
		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("StaticMesh");
#else
		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >(m_name.c_str());
#endif
		
		renderBlock->distance = distance;
		renderBlock->program = program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_renderMesh->getIndexBuffer();
		renderBlock->vertexBuffer = m_renderMesh->getVertexBuffer();
		renderBlock->primitives = &meshParts[i->meshPart].primitives;

		renderBlock->programParams->beginParameters(renderContext);
		
		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			m_shader->getCurrentPriority(),
			transform,
			boundingBox
		);
		
		if (parameterCallback)
			parameterCallback->setParameters(renderBlock->programParams);

		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(
			m_shader->getCurrentPriority(),
			renderBlock
		);
	}
}

	}
}
