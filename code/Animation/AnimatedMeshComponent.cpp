/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "Animation/AnimatedMeshComponent.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonComponent.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/IPoseController.h"
#include "Animation/Joint.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/JobManager.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Render/Buffer.h"
#include "Render/IAccelerationStructure.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/Context/RenderBlock.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/World.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::animation
{
	namespace
	{

const render::Handle s_techniqueVelocityWrite(L"World_VelocityWrite");
const render::Handle s_handleWorld_ShadowWrite(L"World_ShadowWrite");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimatedMeshComponent", AnimatedMeshComponent, mesh::MeshComponent)

AnimatedMeshComponent::AnimatedMeshComponent(
	const Transform& transform,
	const resource::Proxy< mesh::SkinnedMesh >& mesh,
	render::IRenderSystem* renderSystem
)
:	mesh::MeshComponent()
,	m_mesh(mesh)
,	m_index(0)
{
	const uint32_t skinJointCount = m_mesh->getJointCount();

	// Create buffer to contain the joint matrix palette.
	m_jointBuffer = mesh::SkinnedMesh::createJointBuffer(renderSystem, skinJointCount);

	// Create skin buffers.
	m_skinBuffer[0] = m_mesh->createSkinBuffer(renderSystem);
	m_skinBuffer[1] = m_mesh->createSkinBuffer(renderSystem);

	m_jointInverseTransforms.resize(skinJointCount, Transform::identity());
	m_poseTransforms[0].resize(skinJointCount, Transform::identity());
	m_poseTransforms[1].resize(skinJointCount, Transform::identity());

	// Create our instance's acceleration structure.
	m_rtAccelerationStructure = m_mesh->createAccelerationStructure(renderSystem);
}

void AnimatedMeshComponent::destroy()
{
	m_mesh.clear();
	safeDestroy(m_jointBuffer);
	safeDestroy(m_skinBuffer[0]);
	safeDestroy(m_skinBuffer[1]);
	safeDestroy(m_rtwInstance);
	safeDestroy(m_rtAccelerationStructure);
	mesh::MeshComponent::destroy();
}

void AnimatedMeshComponent::setOwner(world::Entity* owner)
{
	mesh::MeshComponent::setOwner(owner);

	m_jointRemap.resize(0);

	if (owner != nullptr)
	{
		auto skeletonComponent = owner->getComponent< SkeletonComponent >();
		if (skeletonComponent && skeletonComponent->getSkeleton())
		{
			auto skeleton = skeletonComponent->getSkeleton();
			const auto& jointTransforms = skeletonComponent->getJointTransforms();

			m_jointRemap.resize(skeleton->getJointCount());

			const auto& jointMap = m_mesh->getJointMap();
			for (uint32_t i = 0; i < skeleton->getJointCount(); ++i)
			{
				const Joint* joint = skeleton->getJoint(i);
				auto it = jointMap.find(joint->getName());
				if (it == jointMap.end())
				{
					m_jointRemap[i] = -1;
					continue;
				}
				m_jointRemap[i] = it->second;
				m_jointInverseTransforms[it->second] = jointTransforms[i].inverse();
			}
		}
	}
}

void AnimatedMeshComponent::setWorld(world::World* world)
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

void AnimatedMeshComponent::setState(const world::EntityState& state, const world::EntityState& mask)
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

void AnimatedMeshComponent::setTransform(const Transform& transform)
{
	MeshComponent::setTransform(transform);

	if (m_rtwInstance)
		m_rtwInstance->setTransform(transform);
}

Aabb3 AnimatedMeshComponent::getBoundingBox() const
{
	auto skeletonComponent = m_owner->getComponent< SkeletonComponent >();
	return skeletonComponent != nullptr ? skeletonComponent->getBoundingBox() : m_mesh->getBoundingBox();
}

void AnimatedMeshComponent::update(const world::UpdateParams& update)
{
	// Always ensure skin arrays are same size as mesh joints.
	const size_t skinJointCount = m_mesh->getJointCount();
	m_jointInverseTransforms.resize(skinJointCount);
	m_poseTransforms[0].resize(skinJointCount, Transform::identity());
	m_poseTransforms[1].resize(skinJointCount, Transform::identity());
	m_index = 1 - m_index;

	// Calculate skinning transforms.
	auto skeletonComponent = m_owner->getComponent< SkeletonComponent >();
	if (skeletonComponent && skeletonComponent->getSkeleton())
	{
		// Ensure all transforms are calculated.
		skeletonComponent->synchronize();

		auto skeleton = skeletonComponent->getSkeleton();

		const auto& jointTransforms = skeletonComponent->getJointTransforms();
		const auto& poseTransforms = skeletonComponent->getPoseTransforms();

		if (!poseTransforms.empty())
		{
			const size_t skeletonJointCount = jointTransforms.size();
			const size_t skinJointCount = m_mesh->getJointCount();
			for (size_t i = 0; i < skeletonJointCount; ++i)
			{
				const int32_t jointIndex = m_jointRemap[i];
				if (jointIndex >= 0 && jointIndex < int32_t(skinJointCount))
					m_poseTransforms[m_index][jointIndex] = poseTransforms[i];
			}
		}
	}

	mesh::MeshComponent::update(update);
}

void AnimatedMeshComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	const Scalar interval(worldRenderView.getInterval());
	const Transform worldTransform = m_transform.get(interval);

	const bool firstInFrame = ((worldRenderPass.getPassFlags() & world::IWorldRenderPass::First) != 0 && worldRenderView.getIndex() == 0);
	const bool supportTechnique = (m_mesh->supportTechnique(worldRenderPass.getTechnique()));

	bool isVisible = false;
	float distance = 0.0f;

	if (firstInFrame || supportTechnique)
	{
		isVisible = worldRenderView.isBoxVisible(
			m_mesh->getBoundingBox(),
			worldTransform,
			distance
		);
	}

	if (firstInFrame)
	{
		m_lastWorldTransform[1] = m_lastWorldTransform[0];
		m_lastWorldTransform[0] = worldTransform;

		const auto& poseTransformsLastUpdate = m_poseTransforms[1 - m_index];
		const auto& poseTransformsCurrentUpdate = m_poseTransforms[m_index];

		auto skeletonComponent = m_owner->getComponent< SkeletonComponent >();
		const auto& jointTransforms = skeletonComponent->getJointTransforms();

		if (isVisible)
		{
			// Interpolate between updates to get current build skin transforms.
			if (poseTransformsCurrentUpdate.size() > 0)
			{
				mesh::SkinnedMesh::JointData* jointData = (mesh::SkinnedMesh::JointData*)m_jointBuffer->lock();
				for (uint32_t i = 0; i < poseTransformsCurrentUpdate.size(); ++i)
				{
					const Transform poseTransform = lerp(poseTransformsLastUpdate[i], poseTransformsCurrentUpdate[i], interval);
					const Transform skinTransform = poseTransform * m_jointInverseTransforms[i];
					skinTransform.translation().storeAligned(jointData->translation);
					skinTransform.rotation().e.storeAligned(jointData->rotation);
					jointData++;
				}
				m_jointBuffer->unlock();
			}

			// Update skin.
			std::swap(m_skinBuffer[0], m_skinBuffer[1]);
			m_mesh->buildSkin(context.getRenderContext(), m_jointBuffer, m_skinBuffer[0]);
			if (m_rtwInstance)
			{
				m_mesh->buildAccelerationStructure(context.getRenderContext(), m_skinBuffer[0], m_rtAccelerationStructure);
				m_rtwInstance->setDirty();
			}
		}
	}

	if (supportTechnique && isVisible)
	{
		m_mesh->build(
			context.getRenderContext(),
			worldRenderPass,
			m_lastWorldTransform[1],
			worldTransform,
			m_lastIsVisible ? m_skinBuffer[1] : m_skinBuffer[0],
			m_skinBuffer[0],
			distance,
			getParameterCallback()
		);
	}

	if (firstInFrame)
		m_lastIsVisible = isVisible;
}

bool AnimatedMeshComponent::getSkinTransform(render::handle_t jointName, Transform& outTransform) const
{
	auto skeletonComponent = m_owner->getComponent< SkeletonComponent >();
	if (!skeletonComponent)
		return false;

	auto skeleton = skeletonComponent->getSkeleton();
	if (!skeleton)
		return false;

	uint32_t index;
	if (!skeleton->findJoint(jointName, index))
		return false;

	if (index >= m_jointRemap.size())
		return false;

	const int skinIndex = m_jointRemap[index];
	if (skinIndex < 0)
		return false;

	outTransform = m_poseTransforms[m_index][skinIndex] * m_jointInverseTransforms[skinIndex];
	return true;
}

}
