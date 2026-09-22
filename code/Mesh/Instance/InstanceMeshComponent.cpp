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
#include "Render/IAccelerationStructure.h"
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
	releaseDeformSlot();
	safeDestroy(m_cullingInstance);
	safeDestroy(m_rtwInstance);
	m_mesh.clear();
	MeshComponent::destroy();
}

void InstanceMeshComponent::setWorld(world::World* world)
{
	MeshComponent::setWorld(world);
	releaseDeformSlot();
	safeDestroy(m_cullingInstance);
	safeDestroy(m_rtwInstance);
}

void InstanceMeshComponent::setState(const world::EntityState& state, const world::EntityState& mask, bool includeChildren)
{
	m_dynamic = state.dynamic;

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
		if (!m_cullingInstance)
		{
			world::CullingComponent* culling = m_world->getComponent< world::CullingComponent >();
			m_cullingInstance = culling->createInstance(m_mesh, (intptr_t)m_mesh.getResource(), m_dynamic);
			m_cullingInstance->setTransform(m_transform->currentRender);
		}
		else
			m_cullingInstance->setDynamic(m_dynamic);
	}
	else
	{
		releaseDeformSlot();
		safeDestroy(m_cullingInstance);
		safeDestroy(m_rtwInstance);
	}
}

void InstanceMeshComponent::setTransform(const Transform& transform)
{
	MeshComponent::setTransform(transform);
}

Aabb3 InstanceMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void InstanceMeshComponent::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView)
{
	// Recreate RT and culling if mesh has been reloaded.
	if (m_mesh.changed())
	{
		releaseDeformSlot();

		if (m_rtwInstance)
		{
			safeDestroy(m_rtwInstance);

			world::RTWorldComponent* rtw = m_world->getComponent< world::RTWorldComponent >();
			if (rtw != nullptr && m_mesh->getAccelerationStructure() != nullptr)
			{
				m_rtwInstance = rtw->createInstance(m_mesh->getAccelerationStructure(), m_mesh->getRTVertexAttributes());
				m_rtwInstance->setTransform(m_transform->currentRender);
			}
		}
		if (m_cullingInstance)
		{
			safeDestroy(m_cullingInstance);

			world::CullingComponent* culling = m_world->getComponent< world::CullingComponent >();
			m_cullingInstance = culling->createInstance(m_mesh, (intptr_t)m_mesh.getResource(), m_dynamic);
			m_cullingInstance->setTransform(m_transform->currentRender);
		}
		m_mesh.consume();
	}

	if (m_cullingInstance)
	{
		if (!fuzzyEqual(m_transform->currentRender, m_cullingInstance->transform))
		{
			if (m_rtwInstance)
				m_rtwInstance->setTransform(m_transform->currentRender);

			m_cullingInstance->setTransform(m_transform->currentRender);
		}
	}
}

bool InstanceMeshComponent::setupDeform(const world::WorldRenderView& worldRenderView)
{
	T_ASSERT(m_mesh->haveDeform());

	// Only instances which are culled, i.e. visible, deform.
	const bool inRange = (m_cullingInstance != nullptr) && DeformMesh::isWithinDeformDistance(m_transform->currentRender, m_mesh->getBoundingBox(), worldRenderView.getEyePosition());
	if (inRange && m_deformSlot < 0)
	{
		m_deformSlot = m_mesh->allocateDeformSlot(this);
		m_deformSlotMesh = m_mesh.getResource();
		m_deformSlotNew = true;
	}
	else if (!inRange && m_deformSlot >= 0)
		releaseDeformSlot();

	// Deformed instances move every frame regardless of their transforms.
	if (m_deformSlot >= 0 && m_cullingInstance)
		m_cullingInstance->setVelocityDirty();

	return m_deformSlot >= 0;
}

bool InstanceMeshComponent::getBatchIndex(uint32_t& outBatchIndex) const
{
	if (!m_cullingInstance)
		return false;
	outBatchIndex = m_cullingInstance->batchIndex;
	return true;
}

void InstanceMeshComponent::buildDeform(const world::WorldRenderView& worldRenderView, render::RenderContext* renderContext)
{
	T_ASSERT(m_deformSlot >= 0);
	m_mesh->buildDeformSlot(renderContext, worldRenderView, m_deformSlot, m_transform->currentRender, m_mesh->getShader(), m_parameterCallback, m_deformSlotNew);
	m_deformSlotNew = false;
}

void InstanceMeshComponent::buildDeformAccelerationStructure(const world::WorldRenderView& worldRenderView, render::RenderContext* renderContext)
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

void InstanceMeshComponent::releaseDeformSlot()
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
