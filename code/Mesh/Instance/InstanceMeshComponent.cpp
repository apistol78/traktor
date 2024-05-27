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
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/World.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMeshComponent", InstanceMeshComponent, MeshComponent)

InstanceMeshComponent::InstanceMeshComponent(const resource::Proxy< InstanceMesh >& mesh)
:	m_mesh(mesh)
{
	m_mesh.consume();
}

InstanceMeshComponent::~InstanceMeshComponent()
{
	// Need to call destroy here since editor doesn't always call destroy on components
	// but instead rely on reference counting to do the cleanup.
	destroy();
}

void InstanceMeshComponent::destroy()
{
	T_FATAL_ASSERT(m_cullingInstance == nullptr);
	m_mesh.clear();
	MeshComponent::destroy();
}

void InstanceMeshComponent::setWorld(world::World* world)
{
	// Remove from last world.
	if (m_world != nullptr && m_cullingInstance != nullptr)
	{
		world::CullingComponent* culling = m_world->getComponent< world::CullingComponent >();
		culling->releaseInstance(m_cullingInstance);
	}

	// Add to new world.
	if (world != nullptr)
	{
		T_FATAL_ASSERT(m_cullingInstance == nullptr);
		world::CullingComponent* culling = world->getComponent< world::CullingComponent >();
		m_cullingInstance = culling->allocateInstance(this, (intptr_t)m_mesh.getResource());
	}

	m_world = world;
}

void InstanceMeshComponent::setState(const world::EntityState& state, const world::EntityState& mask)
{
	const bool visible = (state.visible && mask.visible);
	if (visible)
	{
		if (!m_cullingInstance)
		{
			world::CullingComponent* culling = m_world->getComponent< world::CullingComponent >();
			m_cullingInstance = culling->allocateInstance(this, (intptr_t)m_mesh.getResource());
			m_cullingInstance->setTransform(m_transform.get0());
		}
	}
	else
	{
		if (m_cullingInstance)
		{
			world::CullingComponent* culling = m_world->getComponent< world::CullingComponent >();
			culling->releaseInstance(m_cullingInstance);
		}
	}
}

void InstanceMeshComponent::setTransform(const Transform& transform)
{
	MeshComponent::setTransform(transform);

	if (m_cullingInstance)
		m_cullingInstance->setTransform(transform);
}

Aabb3 InstanceMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void InstanceMeshComponent::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	// Not used since we're getting called by the culling component when we should build.
}

void InstanceMeshComponent::cullableBuild(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	render::Buffer* instanceBuffer,
	render::Buffer* visibilityBuffer,
	uint32_t start,
	uint32_t count
)
{
	// Draw mesh instances; this method is called for the "first" InstanceMeshComponent using the same ordinal number
	// assuming all other components reference the same mesh.
	if (m_mesh)
		m_mesh->build(
			context,
			worldRenderView,
			worldRenderPass,
			instanceBuffer,
			visibilityBuffer,
			start,
			count
		);
}

}
