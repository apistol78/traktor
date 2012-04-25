#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Static/StaticMesh.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/IWorldRenderPass.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

render::handle_t s_handleUserParameter = 0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StaticMesh", StaticMesh, IMesh)

StaticMesh::StaticMesh()
{
	if (!s_handleUserParameter)
		s_handleUserParameter = render::getParameterHandle(L"UserParameter");
}

const Aabb3& StaticMesh::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

bool StaticMesh::supportTechnique(render::handle_t technique) const
{
	return m_parts.find(technique) != m_parts.end();
}

void StaticMesh::render(
	render::RenderContext* renderContext,
	const world::IWorldRenderPass& worldRenderPass,
	const Transform& worldTransform,
	float distance,
	float userParameter,
	const IMeshParameterCallback* parameterCallback
)
{
	SmallMap< render::handle_t, std::vector< Part > >::const_iterator it = m_parts.find(worldRenderPass.getTechnique());
	T_ASSERT (it != m_parts.end());

	const std::vector< render::Mesh::Part >& meshParts = m_mesh->getParts();
	for (std::vector< Part >::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
	{
		m_shader->setTechnique(i->shaderTechnique);

		worldRenderPass.setShaderCombination(
			m_shader,
			worldTransform.toMatrix44(),
			getBoundingBox()
		);

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
		renderBlock->indexBuffer = m_mesh->getIndexBuffer();
		renderBlock->vertexBuffer = m_mesh->getVertexBuffer();
		renderBlock->primitives = &meshParts[i->meshPart].primitives;

		renderBlock->programParams->beginParameters(renderContext);
		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			worldTransform.toMatrix44(),
			getBoundingBox()
		);
		renderBlock->programParams->setFloatParameter(s_handleUserParameter, userParameter);
		if (parameterCallback)
			parameterCallback->setParameters(renderBlock->programParams);
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(
			i->opaque ? render::RfOpaque : render::RfAlphaBlend,
			renderBlock
		);
	}
}

	}
}
