/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Indoor/IndoorMesh.h"
#include "Mesh/Indoor/IndoorMeshComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.IndoorMeshComponent", IndoorMeshComponent, MeshComponent)

IndoorMeshComponent::IndoorMeshComponent(const resource::Proxy< IndoorMesh >& mesh)
:	m_mesh(mesh)
{
}

void IndoorMeshComponent::destroy()
{
	m_mesh.clear();
	MeshComponent::destroy();
}

Aabb3 IndoorMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void IndoorMeshComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	if (!m_mesh->supportTechnique(worldRenderPass.getTechnique()))
		return;

	const Transform transform = m_transform.get(worldRenderView.getInterval());
	const Aabb3 boundingBox = m_mesh->getBoundingBox();

	float distance = 0.0f;
	if (!worldRenderView.isBoxVisible(
		boundingBox,
		transform,
		distance
	))

	m_mesh->build(
		context.getRenderContext(),
		worldRenderView,
		worldRenderPass,
		transform,
		distance,
		m_parameterCallback
	);
}

}
