/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshComponent.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/World.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{
	namespace
	{

static const render::Handle s_techniqueVelocityWrite(L"World_VelocityWrite");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StaticMeshComponent", StaticMeshComponent, MeshComponent)

StaticMeshComponent::StaticMeshComponent(const resource::Proxy< StaticMesh >& mesh)
:	m_mesh(mesh)
,	m_lastTransform(Transform::identity())
{
}

void StaticMeshComponent::destroy()
{
	m_mesh.clear();
	MeshComponent::destroy();
}

void StaticMeshComponent::setOwner(world::Entity* owner)
{
	if (owner != nullptr)
		m_lastTransform = owner->getTransform();
	MeshComponent::setOwner(owner);
}

void StaticMeshComponent::setWorld(world::World* world)
{
	// Remove from last world.
	if (m_world != nullptr && m_rtwInstance != nullptr)
	{
		world::RTWorldComponent* rtw = m_world->getComponent< world::RTWorldComponent >();
		rtw->releaseInstance(m_rtwInstance);
	}

	// Add to new world.
	if (world != nullptr)
	{
		T_FATAL_ASSERT(m_rtwInstance == nullptr);
		world::RTWorldComponent* rtw = world->getComponent< world::RTWorldComponent >();
		if (rtw != nullptr)
			m_rtwInstance = rtw->allocateInstance(m_mesh->getAccelerationStructure(), nullptr);
	}

	m_world = world;
}

void StaticMeshComponent::setState(const world::EntityState& state, const world::EntityState& mask)
{
	const bool visible = (state.visible && mask.visible);
	if (visible)
	{
		if (!m_rtwInstance)
		{
			world::RTWorldComponent* rtw = m_world->getComponent< world::RTWorldComponent >();
			if (rtw != nullptr)
			{
				m_rtwInstance = rtw->allocateInstance(m_mesh->getAccelerationStructure(), nullptr);
				m_rtwInstance->setTransform(m_transform.get0());
			}
		}
	}
	else
	{
		if (m_rtwInstance)
		{
			world::RTWorldComponent* rtw = m_world->getComponent< world::RTWorldComponent >();
			rtw->releaseInstance(m_rtwInstance);
		}
	}
}

void StaticMeshComponent::setTransform(const Transform& transform)
{
	MeshComponent::setTransform(transform);

	if (m_rtwInstance)
		m_rtwInstance->setTransform(transform);
}

Aabb3 StaticMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void StaticMeshComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	const StaticMesh::techniqueParts_t* techniqueParts = m_mesh->findTechniqueParts(worldRenderPass.getTechnique());
	if (!techniqueParts)
		return;

	const Transform worldTransform = m_transform.get(worldRenderView.getInterval());

	// Skip rendering velocities if mesh hasn't moved since last frame.
	if (worldRenderPass.getTechnique() == s_techniqueVelocityWrite)
	{
		if (worldTransform == m_lastTransform)
			return;
	}

	float distance = 0.0f;
	if (!worldRenderView.isBoxVisible(
		m_mesh->getBoundingBox(),
		worldTransform,
		distance
	))
		return;

	m_mesh->build(
		context.getRenderContext(),
		worldRenderPass,
		*techniqueParts,
		m_lastTransform,
		worldTransform,
		distance,
		m_parameterCallback
	);

	// Save last rendered transform so we can properly write velocities next frame.
	if (worldRenderPass.getTechnique() == s_techniqueVelocityWrite)
		m_lastTransform = worldTransform;
}

}
