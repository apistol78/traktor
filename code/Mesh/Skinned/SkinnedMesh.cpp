#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Render/IRenderSystem.h"
#include "Render/StructElement.h"
#include "Render/StructBuffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/IWorldRenderPass.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

const render::Handle s_handleLastJoints(L"Mesh_LastJoints");
const render::Handle s_handleJoints(L"Mesh_Joints");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.SkinnedMesh", SkinnedMesh, IMesh)

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
	render::StructBuffer* lastJointTransforms,
	render::StructBuffer* jointTransforms,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	auto it = m_parts.find(worldRenderPass.getTechnique());
	T_ASSERT(it != m_parts.end());

	const auto& meshParts = m_mesh->getParts();
	for (const auto& part : it->second)
	{
		auto permutation = worldRenderPass.getPermutation(m_shader);
		permutation.technique = part.shaderTechnique;
		auto sp = m_shader->getProgram(permutation);
		if (!sp)
			continue;

		render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >(L"SkinnedMesh");
		renderBlock->distance = distance;
		renderBlock->program = sp.program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_mesh->getIndexBuffer();
		renderBlock->vertexBuffer = m_mesh->getVertexBuffer();
		renderBlock->primitives = meshParts[part.meshPart].primitives;

		renderBlock->programParams->beginParameters(renderContext);
		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			lastWorldTransform,
			worldTransform
		);

		if (parameterCallback)
			parameterCallback->setParameters(renderBlock->programParams);

		renderBlock->programParams->setBufferViewParameter(s_handleLastJoints, lastJointTransforms->getBufferView());
		renderBlock->programParams->setBufferViewParameter(s_handleJoints, jointTransforms->getBufferView());

		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(
			sp.priority,
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

Ref< render::StructBuffer > SkinnedMesh::createJointBuffer(render::IRenderSystem* renderSystem, uint32_t jointCount)
{
	AlignedVector< render::StructElement > jointDataStruct;
	jointDataStruct.push_back({ render::DtFloat4, offsetof(JointData, translation) });
	jointDataStruct.push_back({ render::DtFloat4, offsetof(JointData, rotation) });
	T_FATAL_ASSERT(sizeof(JointData) == render::getStructSize(jointDataStruct));

	return renderSystem->createStructBuffer(jointDataStruct, jointCount * sizeof(JointData), true);
}

	}
}
