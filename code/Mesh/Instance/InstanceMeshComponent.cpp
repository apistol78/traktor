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
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMeshComponent", InstanceMeshComponent, MeshComponent)

InstanceMeshComponent::InstanceMeshComponent(const resource::Proxy< InstanceMesh >& mesh)
:	m_mesh(mesh)
{
	m_meshInstance = m_mesh->allocateInstance();
}

InstanceMeshComponent::~InstanceMeshComponent()
{
	// Need to call destroy here since editor doesn't always call destroy on components
	// but instead rely on reference counting to do the cleanup.
	destroy();
}

void InstanceMeshComponent::destroy()
{
	if (m_mesh && m_meshInstance != nullptr)
		m_mesh->releaseInstance(m_meshInstance);
	m_mesh.clear();
	MeshComponent::destroy();
}

void InstanceMeshComponent::setTransform(const Transform& transform)
{
	MeshComponent::setTransform(transform);
	m_meshInstance->setTransform(transform);
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
