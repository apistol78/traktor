/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/IWorldRenderPass.h"

namespace traktor::mesh
{
	namespace
	{

const render::Handle s_handleSkinBuffer(L"Mesh_SkinBuffer");
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
	render::Buffer* lastJointTransforms,
	render::Buffer* jointTransforms,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	auto it = m_parts.find(worldRenderPass.getTechnique());
	T_ASSERT(it != m_parts.end());

	// Setup the parameters; these are shared for all technique parts.
	auto programParams = renderContext->alloc< render::ProgramParameters >();
	programParams->beginParameters(renderContext);

	worldRenderPass.setProgramParameters(
		programParams,
		lastWorldTransform,
		worldTransform
	);

	if (parameterCallback)
		parameterCallback->setParameters(programParams);

	programParams->setBufferViewParameter(s_handleSkinBuffer, m_mesh->getAuxBuffer()->getBufferView());
	programParams->setBufferViewParameter(s_handleLastJoints, lastJointTransforms->getBufferView());
	programParams->setBufferViewParameter(s_handleJoints, jointTransforms->getBufferView());

	programParams->endParameters(renderContext);

	// Draw each technique part.
	const auto& meshParts = m_mesh->getParts();
	for (const auto& part : it->second)
	{
		auto permutation = worldRenderPass.getPermutation(m_shader);
		permutation.technique = part.shaderTechnique;
		auto sp = m_shader->getProgram(permutation);
		if (!sp)
			continue;

		render::SimpleRenderBlock* renderBlock = renderContext->allocNamed< render::SimpleRenderBlock >(L"SkinnedMesh");
		renderBlock->distance = distance;
		renderBlock->program = sp.program;
		renderBlock->programParams = programParams;
		renderBlock->indexBuffer = m_mesh->getIndexBuffer()->getBufferView();
		renderBlock->indexType = m_mesh->getIndexType();
		renderBlock->vertexBuffer = (m_mesh->getVertexBuffer() != nullptr) ? m_mesh->getVertexBuffer()->getBufferView() : nullptr;
		renderBlock->vertexLayout = m_mesh->getVertexLayout();
		renderBlock->primitives = meshParts[part.meshPart].primitives;

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

Ref< render::Buffer > SkinnedMesh::createJointBuffer(render::IRenderSystem* renderSystem, uint32_t jointCount)
{
	return renderSystem->createBuffer(render::BuStructured, std::max< uint32_t >(jointCount, 1) * sizeof(JointData), true);
}

}
