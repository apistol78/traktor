/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Partition/IPartition.h"
#include "Mesh/Partition/PartitionMesh.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.PartitionMesh", PartitionMesh, IMesh)

const Aabb3& PartitionMesh::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

bool PartitionMesh::supportTechnique(render::handle_t technique) const
{
	return true;
}

void PartitionMesh::build(
	render::RenderContext* renderContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	const Transform& worldTransform,
	float distance,
	const IMeshParameterCallback* parameterCallback
)
{
	if (!m_partition)
		return;

	Matrix44 worldView = worldRenderView.getView() * worldTransform.toMatrix44();

	m_partIndices.resize(0);
	m_partition->traverse(
		worldRenderView.getCullFrustum(),
		worldView,
		worldRenderPass.getTechnique(),
		m_partIndices
	);
	if (m_partIndices.empty())
		return;

	const AlignedVector< render::Mesh::Part >& meshParts = m_mesh->getParts();
	for (auto partIndex : m_partIndices)
	{
		const Part& part = m_parts[partIndex];

		auto permutation = worldRenderPass.getPermutation(m_shader);
		permutation.technique = part.shaderTechnique;
		auto sp = m_shader->getProgram(permutation);
		if (!sp)
			continue;

		Vector4 center = worldView * part.boundingBox.getCenter();
		Scalar distancePart = center.z() + part.boundingBox.getExtent().length();

		auto renderBlock = renderContext->alloc< render::SimpleRenderBlock >(L"PartitionMesh");
		renderBlock->distance = distancePart;
		renderBlock->program = sp.program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_mesh->getIndexBuffer()->getBufferView();
		renderBlock->indexType = m_mesh->getIndexType();
		renderBlock->vertexBuffer = m_mesh->getVertexBuffer()->getBufferView();
		renderBlock->vertexLayout = m_mesh->getVertexLayout();
		renderBlock->primitives = meshParts[part.meshPart].primitives;

		renderBlock->programParams->beginParameters(renderContext);
		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			worldTransform,
			worldTransform
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
