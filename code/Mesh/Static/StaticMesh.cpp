#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Static/StaticMesh.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/WorldRenderView.h"

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

const Aabb& StaticMesh::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void StaticMesh::render(
	render::RenderContext* renderContext,
	const world::WorldRenderView* worldRenderView,
	const Transform& worldTransform,
	const Transform& worldTransformPrevious,
	float distance,
	float userParameter,
	const IMeshParameterCallback* parameterCallback
)
{
	if (!m_shader.validate())
		return;

	std::map< render::handle_t, std::vector< Part > >::const_iterator it = m_parts.find(worldRenderView->getTechnique());
	if (it == m_parts.end())
		return;

	const std::vector< render::Mesh::Part >& meshParts = m_mesh->getParts();
	for (std::vector< Part >::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
	{
		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("StaticMesh");

		renderBlock->distance = distance;
		renderBlock->shader = m_shader;
		renderBlock->shaderParams = renderContext->alloc< render::ShaderParameters >();
		renderBlock->indexBuffer = m_mesh->getIndexBuffer();
		renderBlock->vertexBuffer = m_mesh->getVertexBuffer();
		renderBlock->primitives = &meshParts[i->meshPart].primitives;

		renderBlock->shaderParams->beginParameters(renderContext);
		renderBlock->shaderParams->setTechnique(i->shaderTechnique);
		worldRenderView->setShaderParameters(
			renderBlock->shaderParams,
			worldTransform.toMatrix44(),
			worldTransformPrevious.toMatrix44(),
			getBoundingBox()
		);
		renderBlock->shaderParams->setFloatParameter(s_handleUserParameter, userParameter);
		if (parameterCallback)
			parameterCallback->setParameters(renderBlock->shaderParams);
		renderBlock->shaderParams->endParameters(renderContext);

		renderContext->draw(
			i->opaque ? render::RfOpaque : render::RfAlphaBlend,
			renderBlock
		);
	}
}

	}
}
