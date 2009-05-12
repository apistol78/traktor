#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Context/RenderContext.h"
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.SkinnedMesh", SkinnedMesh, Object)

SkinnedMesh::SkinnedMesh()
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
	const Matrix44& worldTransform,
	const AlignedVector< Matrix44 >& boneTransforms,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	const std::vector< render::Mesh::Part >& parts = m_mesh->getParts();
	T_ASSERT (parts.size() == m_parts.size());

	for (size_t i = 0; i < parts.size(); ++i)
	{
		if (!m_parts[i].material.validate())
			continue;

		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >();

		renderBlock->distance = distance;
		renderBlock->shader = m_parts[i].material;
		renderBlock->shaderParams = renderContext->alloc< render::ShaderParameters >();
		renderBlock->indexBuffer = m_mesh->getIndexBuffer();
		renderBlock->vertexBuffer = m_mesh->getVertexBuffer();
		renderBlock->primitives = &parts[i].primitives;

		renderBlock->shaderParams->beginParameters(renderContext);
		if (parameterCallback)
			parameterCallback->setParameters(renderBlock->shaderParams);
		if (!boneTransforms.empty())
			renderBlock->shaderParams->setMatrixArrayParameter(s_handleBones, &boneTransforms[0], int(boneTransforms.size()));
		worldRenderView->setShaderParameters(renderBlock->shaderParams, worldTransform, getBoundingBox());
		renderBlock->shaderParams->endParameters(renderContext);

		renderBlock->type = m_parts[i].material->isOpaque() ? render::RbtOpaque : render::RbtAlphaBlend;

		renderContext->draw(renderBlock);
	}
}

const std::map< std::wstring, int >& SkinnedMesh::getBoneMap() const
{
	return m_boneMap;
}

	}
}
