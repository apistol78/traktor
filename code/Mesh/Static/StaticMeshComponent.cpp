/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Static/StaticMeshComponent.h"

#include "Core/Misc/SafeDestroy.h"
#include "Mesh/Static/StaticMesh.h"
#include "Render/IAccelerationStructure.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/World.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{
namespace
{

const render::Handle s_techniqueVelocityWrite(L"World_VelocityWrite");

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StaticMeshComponent", StaticMeshComponent, MeshComponent)

StaticMeshComponent::StaticMeshComponent(const resource::Proxy< StaticMesh >& mesh, render::IRenderSystem* renderSystem)
	: m_mesh(mesh)
{
}

void StaticMeshComponent::destroy()
{
	releaseDeformSlot();
	safeDestroy(m_rtwInstance);
	m_mesh.clear();
	MeshComponent::destroy();
}

void StaticMeshComponent::setWorld(world::World* world)
{
	MeshComponent::setWorld(world);
	releaseDeformSlot();
	safeDestroy(m_rtwInstance);
}

void StaticMeshComponent::setState(const world::EntityState& state, const world::EntityState& mask, bool includeChildren)
{
	const bool visible = (m_world != nullptr) && state.visible;
	if (visible)
	{
		if (!m_rtwInstance && m_mesh->getAccelerationStructure() != nullptr)
		{
			world::RTWorldComponent* rtw = m_world->getComponent< world::RTWorldComponent >();
			if (rtw != nullptr)
			{
				m_rtwInstance = rtw->createInstance(m_mesh->getAccelerationStructure(), m_mesh->getRTVertexAttributes());
				m_rtwInstance->setTransform(m_transform->currentRender);
			}
		}
	}
	else
	{
		releaseDeformSlot();
		safeDestroy(m_rtwInstance);
	}
}

void StaticMeshComponent::setTransform(const Transform& transform)
{
	MeshComponent::setTransform(transform);
}

Aabb3 StaticMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void StaticMeshComponent::setup(const world::WorldSetupContext& context, const world::WorldRenderView& worldRenderView)
{
	if (m_rtwInstance && m_transform->currentRender != m_transform->lastRender)
		m_rtwInstance->setTransform(m_transform->currentRender);
}

bool StaticMeshComponent::setupDeform(const world::WorldRenderView& worldRenderView)
{
	T_ASSERT(m_mesh->haveDeform());

	// Hold a slot only while within deform distance; beyond it the mesh is drawn
	// from its vertex stream, which the fading offsets have already converged to.
	const bool inRange = (m_world != nullptr) && DeformMesh::isWithinDeformDistance(m_transform->currentRender, m_mesh->getBoundingBox(), worldRenderView.getEyePosition());
	if (inRange && m_deformSlot < 0)
	{
		m_deformSlot = m_mesh->allocateDeformSlot(this);
		m_deformSlotMesh = m_mesh.getResource();
		m_deformSlotNew = true;
	}
	else if (!inRange && m_deformSlot >= 0)
		releaseDeformSlot();

	return m_deformSlot >= 0;
}

void StaticMeshComponent::buildDeform(const world::WorldRenderView& worldRenderView, render::RenderContext* renderContext)
{
	T_ASSERT(m_deformSlot >= 0);
	m_mesh->buildDeformSlot(renderContext, worldRenderView, m_deformSlot, m_transform->currentRender, m_mesh->getShader(), m_parameterCallback, m_deformSlotNew);
	m_deformSlotNew = false;
}

void StaticMeshComponent::buildDeformAccelerationStructure(const world::WorldRenderView& worldRenderView, render::RenderContext* renderContext)
{
	T_ASSERT(m_deformSlot >= 0);
	if (!m_rtwInstance || m_rtwInstance->parts.empty())
		return;

	// Refitting a structure per instance is expensive in memory, so only the nearest
	// deformed instances trace their deformed geometry; the rest trace the shared,
	// undeformed, structure and give theirs back.
	const render::IAccelerationStructure* blas = nullptr;
	if (DeformMesh::isWithinDeformRayTracingDistance(m_transform->currentRender, m_mesh->getBoundingBox(), worldRenderView.getEyePosition()))
		blas = m_mesh->buildDeformSlotAccelerationStructure(renderContext, m_deformSlot);
	else
		m_mesh->releaseDeformSlotAccelerationStructure(m_deformSlot);

	if (blas == nullptr)
		blas = m_mesh->getAccelerationStructure();

	if (m_rtwInstance->parts[0].blas != blas)
	{
		m_rtwInstance->parts[0].blas = blas;
		m_rtwInstance->setDirty();
	}

	m_rtwInstance->setTransform(m_transform->currentRender);
}

void StaticMeshComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	const StaticMesh::techniqueParts_t* techniqueParts = m_mesh->findTechniqueParts(worldRenderPass.getTechnique());
	if (!techniqueParts)
		return;

	// Skip rendering velocities if mesh hasn't moved since last frame; deformed
	// meshes move regardless of their transform.
	if (worldRenderPass.getTechnique() == s_techniqueVelocityWrite && m_deformSlot < 0)
	{
		if (m_transform->currentRender == m_transform->lastRender)
			return;
	}

	float distance = 0.0f;
	if (!worldRenderView.isBoxVisible(
			m_mesh->getBoundingBox(),
			m_transform->currentRender,
			distance))
		return;

	m_mesh->build(
		context.getRenderContext(),
		worldRenderPass,
		*techniqueParts,
		m_transform->lastRender,
		m_transform->currentRender,
		m_deformSlot,
		distance,
		m_parameterCallback);
}

void StaticMeshComponent::releaseDeformSlot()
{
	if (m_deformSlot < 0)
		return;

	// The slot belongs to the mesh it was allocated from; after a reload the proxy
	// refers to a new mesh with its own, empty, pool which knows nothing of the slot.
	if (m_deformSlotMesh != nullptr && m_deformSlotMesh == m_mesh.getResource())
		m_deformSlotMesh->releaseDeformSlot(m_deformSlot);

	m_deformSlot = -1;
	m_deformSlotMesh = nullptr;
	m_deformSlotNew = false;

	// Back to the shared, undeformed, structure.
	if (m_rtwInstance && !m_rtwInstance->parts.empty())
	{
		m_rtwInstance->parts[0].blas = m_mesh->getAccelerationStructure();
		m_rtwInstance->setDirty();
	}
}

}
