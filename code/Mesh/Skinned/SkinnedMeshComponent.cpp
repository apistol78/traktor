/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshComponent.h"
#include "Render/Buffer.h"
#include "Render/IAccelerationStructure.h"
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.SkinnedMeshComponent", SkinnedMeshComponent, MeshComponent)

SkinnedMeshComponent::SkinnedMeshComponent(const resource::Proxy< SkinnedMesh >& mesh, render::IRenderSystem* renderSystem)
:	m_mesh(mesh)
{
	// Create buffer to contain the joint matrix palette.
	const auto& jointMap = m_mesh->getJointMap();
	m_jointBuffer = SkinnedMesh::createJointBuffer(renderSystem, (uint32_t)jointMap.size());

	// Create skin buffers.
	m_skinBuffer[0] = m_mesh->createSkinBuffer(renderSystem);
	m_skinBuffer[1] = m_mesh->createSkinBuffer(renderSystem);

	// Create our instance's acceleration structure.
	m_rtAccelerationStructure = m_mesh->createAccelerationStructure(renderSystem);
}

void SkinnedMeshComponent::destroy()
{
	m_mesh.clear();
	safeDestroy(m_jointBuffer);
	safeDestroy(m_skinBuffer[0]);
	safeDestroy(m_skinBuffer[1]);
	safeDestroy(m_rtwInstance);
	safeDestroy(m_rtAccelerationStructure);
	MeshComponent::destroy();
}

void SkinnedMeshComponent::setWorld(world::World* world)
{
	// Remove from last world.
	safeDestroy(m_rtwInstance);

	// Add to new world.
	if (world != nullptr)
	{
		T_FATAL_ASSERT(m_rtwInstance == nullptr);
		world::RTWorldComponent* rtw = world->getComponent< world::RTWorldComponent >();
		if (rtw != nullptr)
			m_rtwInstance = rtw->createInstance(m_rtAccelerationStructure, m_mesh->getRTVertexAttributes());
	}

	m_world = world;
}

void SkinnedMeshComponent::setState(const world::EntityState& state, const world::EntityState& mask)
{
	const bool visible = (state.visible && mask.visible);
	if (visible)
	{
		if (!m_rtwInstance)
		{
			world::RTWorldComponent* rtw = m_world->getComponent< world::RTWorldComponent >();
			if (rtw != nullptr)
			{
				m_rtwInstance = rtw->createInstance(m_rtAccelerationStructure, m_mesh->getRTVertexAttributes());
				m_rtwInstance->setTransform(m_transform.get0());
			}
		}
	}
	else
	{
		safeDestroy(m_rtwInstance);
	}
}

void SkinnedMeshComponent::setTransform(const Transform& transform)
{
	MeshComponent::setTransform(transform);

	if (m_rtwInstance)
		m_rtwInstance->setTransform(transform);
}

Aabb3 SkinnedMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void SkinnedMeshComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	if ((worldRenderPass.getPassFlags() & world::IWorldRenderPass::First) != 0 && worldRenderView.getIndex() == 0)
	{
		std::swap(m_skinBuffer[0], m_skinBuffer[1]);
		m_mesh->buildSkin(context.getRenderContext(), m_jointBuffer, m_skinBuffer[0]);
		if (m_rtwInstance)
		{
			m_mesh->buildAccelerationStructure(context.getRenderContext(), m_skinBuffer[0], m_rtAccelerationStructure);
			m_rtwInstance->setDirty();
		}
	}

	if (!m_mesh->supportTechnique(worldRenderPass.getTechnique()))
		return;

	const Transform worldTransform = m_transform.get(worldRenderView.getInterval());
	const Transform lastWorldTransform = m_transform.get(worldRenderView.getInterval() - 1.0f);

	// Skip rendering velocities if mesh hasn't moved since last frame.
	if (worldRenderPass.getTechnique() == s_techniqueVelocityWrite)
	{
		if (worldTransform == lastWorldTransform)
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
		lastWorldTransform,
		worldTransform,
		m_skinBuffer[1],
		m_skinBuffer[0],
		distance,
		m_parameterCallback
	);
}

void SkinnedMeshComponent::setJointTransforms(const AlignedVector< Matrix44 >& jointTransforms_)
{
	SkinnedMesh::JointData* jointData = (SkinnedMesh::JointData*)m_jointBuffer->lock();

	const uint32_t size = (uint32_t)jointTransforms_.size();
	for (uint32_t i = 0; i < size; ++i)
	{
		const Transform joint(jointTransforms_[i]);
		joint.translation().xyz1().storeAligned(jointData[i].translation);
		joint.rotation().e.storeAligned(jointData[i].rotation);
	}

	m_jointBuffer->unlock();
}

}
