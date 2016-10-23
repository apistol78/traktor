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

void SkinnedMesh::render(
	render::RenderContext* renderContext,
	world::IWorldRenderPass& worldRenderPass,
	const Transform& worldTransform,
	const AlignedVector< Vector4 >& jointTransforms,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	SmallMap< render::handle_t, std::vector< Part > >::const_iterator it = m_parts.find(worldRenderPass.getTechnique());
	T_ASSERT (it != m_parts.end());

	const Matrix44 world = worldTransform.toMatrix44();
	const Aabb3 boundingBox = getBoundingBox();

	const std::vector< render::Mesh::Part >& meshParts = m_mesh->getParts();
	for (std::vector< Part >::const_iterator i = it->second.begin(); i != it->second.end(); ++i)
	{
		m_shader->setTechnique(i->shaderTechnique);

		worldRenderPass.setShaderCombination(
			m_shader,
			world,
			boundingBox
		);

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
		renderBlock->primitives = meshParts[i->meshPart].primitives;

		renderBlock->programParams->beginParameters(renderContext);
		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			world,
			boundingBox
		);
		if (parameterCallback)
			parameterCallback->setParameters(renderBlock->programParams);
		if (!jointTransforms.empty())
			renderBlock->programParams->setVectorArrayParameter(s_handleJoints, &jointTransforms[0], int(jointTransforms.size()));
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

const std::map< std::wstring, int >& SkinnedMesh::getJointMap() const
{
	return m_jointMap;
}

	}
}
