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

render::handle_t s_handleJoints = 0;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.SkinnedMesh", SkinnedMesh, IMesh)

SkinnedMesh::SkinnedMesh()
:	m_jointCount(0)
{
	if (!s_handleJoints)
		s_handleJoints = render::getParameterHandle(L"Joints");
}

const Aabb3& SkinnedMesh::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

bool SkinnedMesh::supportTechnique(render::handle_t technique) const
{
	return m_parts.find(technique) != m_parts.end();
}

void SkinnedMesh::build(
	render::RenderContext* renderContext,
	const world::IWorldRenderPass& worldRenderPass,
	const Transform& lastWorldTransform,
	const Transform& worldTransform,
	const AlignedVector< Vector4 >& jointTransforms,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	auto it = m_parts.find(worldRenderPass.getTechnique());
	T_ASSERT(it != m_parts.end());

	const Aabb3& boundingBox = getBoundingBox();

	const AlignedVector< render::Mesh::Part >& meshParts = m_mesh->getParts();
	for (const auto& part : it->second)
	{
		m_shader->setTechnique(part.shaderTechnique);
		worldRenderPass.setShaderCombination(m_shader);

		if (parameterCallback)
			parameterCallback->setCombination(m_shader);

		render::IProgram* program = m_shader->getCurrentProgram();
		if (!program)
			continue;

#if !defined(_DEBUG)
		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >("SkinnedMesh");
#else
		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >(m_name.c_str());
#endif
		renderBlock->distance = distance;
		renderBlock->program = program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_mesh->getIndexBuffer();
		renderBlock->vertexBuffer = m_mesh->getVertexBuffer();
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
		if (!jointTransforms.empty())
			renderBlock->programParams->setVectorArrayParameter(s_handleJoints, jointTransforms.c_ptr(), (int)jointTransforms.size());
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(
			m_shader->getCurrentPriority(),
			renderBlock
		);
	}
}

int32_t SkinnedMesh::getJointCount() const
{
	return m_jointCount;
}

const SmallMap< std::wstring, int >& SkinnedMesh::getJointMap() const
{
	return m_jointMap;
}

	}
}
