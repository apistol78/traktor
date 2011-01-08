#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/IWorldRenderPass.h"

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
	world::IWorldRenderPass& worldRenderPass,
	const Transform& worldTransform,
	const AlignedVector< Vector4 >& boneTransforms,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	if (!m_shader.validate())
		return;

	std::map< render::handle_t, std::vector< Part > >::const_iterator it = m_parts.find(worldRenderPass.getTechnique());
	if (it == m_parts.end())
		return;

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

		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("SkinnedMesh");

		renderBlock->distance = distance;
		renderBlock->program = program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_mesh->getIndexBuffer();
		renderBlock->vertexBuffer = m_mesh->getVertexBuffer();
		renderBlock->primitives = &meshParts[i->meshPart].primitives;

		renderBlock->programParams->beginParameters(renderContext);
		m_shader->setProgramParameters(renderBlock->programParams);
		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			worldTransform.toMatrix44(),
			getBoundingBox()
		);
		if (parameterCallback)
			parameterCallback->setParameters(renderBlock->programParams);
		if (!boneTransforms.empty())
			renderBlock->programParams->setVectorArrayParameter(s_handleBones, &boneTransforms[0], int(boneTransforms.size()));
		renderBlock->programParams->endParameters(renderContext);

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
