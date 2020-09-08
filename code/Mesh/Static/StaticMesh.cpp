#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Static/StaticMesh.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
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

void StaticMesh::build(
	render::RenderContext* renderContext,
	const world::IWorldRenderPass& worldRenderPass,
	const Transform& lastWorldTransform,
	const Transform& worldTransform,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	auto it = m_parts.find(worldRenderPass.getTechnique());
	T_ASSERT(it != m_parts.end());

	Aabb3 boundingBox = m_renderMesh->getBoundingBox();

	const auto& meshParts = m_renderMesh->getParts();
	for (const auto& part : it->second)
	{
		auto permutation = worldRenderPass.getPermutation(m_shader);
		permutation.technique = part.shaderTechnique;
		auto sp = m_shader->getProgram(permutation);
		if (!sp)
			continue;		

		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >(L"StaticMesh");
		renderBlock->distance = distance;
		renderBlock->program = sp.program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_renderMesh->getIndexBuffer();
		renderBlock->vertexBuffer = m_renderMesh->getVertexBuffer();
		renderBlock->primitives = meshParts[part.meshPart].primitives;

		renderBlock->programParams->beginParameters(renderContext);

		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			lastWorldTransform,
			worldTransform,
			boundingBox
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

	}
}
