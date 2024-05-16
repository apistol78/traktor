/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Static/StaticMesh.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/IWorldRenderPass.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StaticMesh", StaticMesh, IMesh)

const Aabb3& StaticMesh::getBoundingBox() const
{
	return m_renderMesh->getBoundingBox();
}

const StaticMesh::techniqueParts_t* StaticMesh::findTechniqueParts(render::handle_t technique) const
{
	auto it = m_techniqueParts.find(technique);
	return it != m_techniqueParts.end() ? &it->second : nullptr;
}

void StaticMesh::build(
	render::RenderContext* renderContext,
	const world::IWorldRenderPass& worldRenderPass,
	const techniqueParts_t& techniqueParts,
	const Transform& lastWorldTransform,
	const Transform& worldTransform,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
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

	programParams->endParameters(renderContext);

	// Draw each technique part.
	const auto& meshParts = m_renderMesh->getParts();
	for (uint32_t i = techniqueParts.first; i < techniqueParts.second; ++i)
	{
		const auto& part = m_parts[i];

		auto permutation = worldRenderPass.getPermutation(m_shader);
		permutation.technique = part.shaderTechnique;
		auto sp = m_shader->getProgram(permutation);
		if (!sp)
			continue;

		render::SimpleRenderBlock* renderBlock = renderContext->allocNamed< render::SimpleRenderBlock >(L"StaticMesh");
		renderBlock->distance = distance;
		renderBlock->program = sp.program;
		renderBlock->programParams = programParams;
		renderBlock->indexBuffer = m_renderMesh->getIndexBuffer()->getBufferView();
		renderBlock->indexType = m_renderMesh->getIndexType();
		renderBlock->vertexBuffer = m_renderMesh->getVertexBuffer()->getBufferView();
		renderBlock->vertexLayout = m_renderMesh->getVertexLayout();
		renderBlock->primitives = meshParts[part.meshPart].primitives;

		renderContext->draw(
			sp.priority,
			renderBlock
		);
	}
}

}
