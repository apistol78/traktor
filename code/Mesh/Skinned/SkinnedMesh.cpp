#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

render::handle_t s_handleBones = 0;
render::handle_t s_handleUserParameter = 0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.SkinnedMesh", SkinnedMesh, IMesh)

SkinnedMesh::SkinnedMesh()
:	m_boneCount(0)
{
	if (!s_handleBones)
		s_handleBones = render::getParameterHandle(L"Bones");
	if (!s_handleUserParameter)
		s_handleUserParameter = render::getParameterHandle(L"UserParameter");
}

const Aabb& SkinnedMesh::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void SkinnedMesh::render(
	render::RenderContext* renderContext,
	const world::WorldRenderView* worldRenderView,
	const Transform& worldTransform,
	const AlignedVector< Matrix44 >& boneTransforms,
	float distance,
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
		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("SkinnedMesh");

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
			worldTransform.toMatrix44(),	// \fixme
			getBoundingBox()
		);
		if (parameterCallback)
			parameterCallback->setParameters(renderBlock->shaderParams);
		if (!boneTransforms.empty())
			renderBlock->shaderParams->setMatrixArrayParameter(s_handleBones, &boneTransforms[0], int(boneTransforms.size()));
		renderBlock->shaderParams->endParameters(renderContext);

		renderContext->draw(
			i->opaque ? render::RfOpaque : render::RfAlphaBlend,
			renderBlock
		);
	}
}

int32_t SkinnedMesh::getBoneCount() const
{
	return m_boneCount;
}

const std::map< std::wstring, int >& SkinnedMesh::getBoneMap() const
{
	return m_boneMap;
}

	}
}
