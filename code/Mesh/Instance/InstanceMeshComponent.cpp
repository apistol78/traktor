/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Instance/InstanceMeshComponent.h"

#include "Core/Misc/SafeDestroy.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/World.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMeshComponent", InstanceMeshComponent, MeshComponent)

InstanceMeshComponent::InstanceMeshComponent(const resource::Proxy< InstanceMesh >& mesh)
	: m_mesh(mesh)
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
	safeDestroy(m_cullingInstance);
	safeDestroy(m_rtwInstance);
	m_mesh.clear();
	MeshComponent::destroy();
}

void InstanceMeshComponent::setWorld(world::World* world)
{
	safeDestroy(m_cullingInstance);
	safeDestroy(m_rtwInstance);
	m_world = world;
}

void InstanceMeshComponent::setState(const world::EntityState& state, const world::EntityState& mask, bool includeChildren)
{
	const bool visible = (m_world != nullptr) && state.visible;
	if (visible)
	{
		if (!m_rtwInstance)
		{
			world::RTWorldComponent* rtw = m_world->getComponent< world::RTWorldComponent >();
			if (rtw != nullptr)
			{
				m_rtwInstance = rtw->createInstance(m_mesh->getAccelerationStructure(), m_mesh->getRTVertexAttributes());
				m_rtwInstance->setTransform(m_transform.get0());
			}
		}
		if (!m_cullingInstance)
		{
			world::CullingComponent* culling = m_world->getComponent< world::CullingComponent >();
			m_cullingInstance = culling->createInstance(m_mesh, (intptr_t)m_mesh.getResource());
			m_cullingInstance->setTransform(m_transform.get0());
		}
	}
	else
	{
		safeDestroy(m_cullingInstance);
		safeDestroy(m_rtwInstance);
	}
}

void InstanceMeshComponent::setTransform(const Transform& transform)
{
	MeshComponent::setTransform(transform);

	if (m_rtwInstance)
		m_rtwInstance->setTransform(transform);
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
	const world::IWorldRenderPass& worldRenderPass)
{
	// Recreate RT and culling if mesh has been reloaded.
	if (m_mesh.changed())
	{
		if (m_rtwInstance)
		{
			safeDestroy(m_rtwInstance);

			world::RTWorldComponent* rtw = m_world->getComponent< world::RTWorldComponent >();
			if (rtw != nullptr)
			{
				m_rtwInstance = rtw->createInstance(m_mesh->getAccelerationStructure(), m_mesh->getRTVertexAttributes());
				m_rtwInstance->setTransform(m_transform.get0());
			}
		}
		if (m_cullingInstance)
		{
			safeDestroy(m_cullingInstance);

			world::CullingComponent* culling = m_world->getComponent< world::CullingComponent >();
			m_cullingInstance = culling->createInstance(m_mesh, (intptr_t)m_mesh.getResource());
			m_cullingInstance->setTransform(m_transform.get0());
		}
		m_mesh.consume();
	}
}

}
