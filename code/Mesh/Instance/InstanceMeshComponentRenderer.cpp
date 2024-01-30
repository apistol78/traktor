/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshComponent.h"
#include "Mesh/Instance/InstanceMeshComponentRenderer.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{
	namespace
	{

static const render::Handle s_techniqueVelocityWrite(L"World_VelocityWrite");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMeshComponentRenderer", InstanceMeshComponentRenderer, world::IEntityRenderer)

const TypeInfoSet InstanceMeshComponentRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< InstanceMeshComponent >();
}

void InstanceMeshComponentRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void InstanceMeshComponentRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void InstanceMeshComponentRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto meshComponent = static_cast< InstanceMeshComponent* >(renderable);
	auto mesh = meshComponent->getMesh();

	if (!mesh->supportTechnique(worldRenderPass.getTechnique()))
		return;

	const Aabb3 boundingBox = meshComponent->getBoundingBox();
	const Transform transform = meshComponent->getTransform().get(worldRenderView.getInterval());

	float distance = 0.0f;
	if (!worldRenderView.isBoxVisible(
		boundingBox,
		transform,
		distance
	))
		return;

	const Transform transformLast = meshComponent->getTransform().get(worldRenderView.getInterval() - 1.0f);

	// Skip rendering velocities if mesh hasn't moved since last frame.
	if (worldRenderPass.getTechnique() == s_techniqueVelocityWrite)
	{
		if (transform == transformLast)
			return;
	}

	m_meshInstances[mesh].push_back(InstanceMesh::RenderInstance(
		packInstanceMeshData(transform),
		packInstanceMeshData(transformLast),
		distance
	));
}

void InstanceMeshComponentRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	for (auto& it : m_meshInstances)
	{
		if (it.second.empty())
			continue;

		it.first->build(
			context.getRenderContext(),
			worldRenderPass,
			it.second,
			nullptr
		);

		it.second.resize(0);
	}
}

}
