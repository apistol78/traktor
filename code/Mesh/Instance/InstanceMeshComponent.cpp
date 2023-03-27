/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/MeshCulling.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMeshComponent", InstanceMeshComponent, MeshComponent)

InstanceMeshComponent::InstanceMeshComponent(const resource::Proxy< InstanceMesh >& mesh)
:	m_mesh(mesh)
{
}

void InstanceMeshComponent::destroy()
{
	m_mesh.clear();
	MeshComponent::destroy();
}

Aabb3 InstanceMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void InstanceMeshComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	T_ASSERT_M(0, L"Forgot to register InstanceMeshComponentRenderer?");
}

}
