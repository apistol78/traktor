/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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
#include "Mesh/MeshCulling.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Render/Buffer.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
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
	render::IRenderSystem* renderSystem,
	bool screenSpaceCulling
)
:	mesh::MeshComponent(screenSpaceCulling)
,	m_mesh(mesh)
,	m_index(0)
{
	const uint32_t skinJointCount = m_mesh->getJointCount();

	m_jointBuffers[0] = mesh::SkinnedMesh::createJointBuffer(renderSystem, skinJointCount);
	m_jointBuffers[1] = mesh::SkinnedMesh::createJointBuffer(renderSystem, skinJointCount);

	m_skinTransforms[0].resize(skinJointCount * 2, Vector4::origo());
	m_skinTransforms[1].resize(skinJointCount * 2, Vector4::origo());
}

void AnimatedMeshComponent::destroy()
{
	safeDestroy(m_jointBuffers[1]);
	safeDestroy(m_jointBuffers[0]);

	mesh::MeshComponent::destroy();
}

void AnimatedMeshComponent::setOwner(world::Entity* owner)
{
	mesh::MeshComponent::setOwner(owner);

	m_jointRemap.resize(0);

	auto skeletonComponent = owner->getComponent< SkeletonComponent >();
	if (skeletonComponent && skeletonComponent->getSkeleton())
	{
		auto skeleton = skeletonComponent->getSkeleton();

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
		}
	}
}

Aabb3 AnimatedMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void AnimatedMeshComponent::update(const world::UpdateParams& update)
{
	// Always ensure skin arrays are same size as mesh joints.
	const size_t skinJointCount = m_mesh->getJointCount();
	m_skinTransforms[0].resize(skinJointCount * 2, Vector4::origo());
	m_skinTransforms[1].resize(skinJointCount * 2, Vector4::origo());
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

			// Calculate skin transforms in delta space.
			for (size_t i = 0; i < skeletonJointCount; ++i)
			{
				const int32_t jointIndex = m_jointRemap[i];
				if (jointIndex >= 0 && jointIndex < int32_t(skinJointCount))
				{
					const Transform skinTransform = poseTransforms[i] * jointTransforms[i].inverse();
					m_skinTransforms[m_index][jointIndex * 2 + 0] = skinTransform.rotation().e;
					m_skinTransforms[m_index][jointIndex * 2 + 1] = skinTransform.translation().xyz1();
				}
			}
		}
	}

	mesh::MeshComponent::update(update);
}

void AnimatedMeshComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	if (!m_mesh->supportTechnique(worldRenderPass.getTechnique()))
		return;

	const Scalar interval(worldRenderView.getInterval());
	const Transform worldTransform = m_transform.get(interval);
	const Transform lastWorldTransform = m_transform.get(interval - 1.0_simd);

	float distance = 0.0f;
	if (!mesh::isMeshVisible(
		m_mesh->getBoundingBox(),
		worldRenderView.getCullFrustum(),
		worldRenderView.getView() * worldTransform.toMatrix44(),
		worldRenderView.getProjection(),
		m_screenSpaceCulling ? 0.0001f : 0.0f,
		distance
	))
		return;

	auto jointBufferLast = m_jointBuffers[0];
	auto jointBufferCurrent = m_jointBuffers[1];

	// Update joint buffers only for first pass of frame, buffers are implicitly double buffered
	// and cannot be updated multiple times per frame.
	if ((worldRenderPass.getPassFlags() & world::IWorldRenderPass::First) != 0)
	{
		const auto& skinTransformsLastUpdate = m_skinTransforms[1 - m_index];
		const auto& skinTransformsCurrentUpdate = m_skinTransforms[m_index];

		// Interpolate between updates to get current build skin transforms.
		mesh::SkinnedMesh::JointData* jointData = (mesh::SkinnedMesh::JointData*)jointBufferCurrent->lock();
		for (uint32_t i = 0; i < skinTransformsCurrentUpdate.size(); i += 2)
		{
			auto translation = lerp(skinTransformsLastUpdate[i + 1], skinTransformsCurrentUpdate[i + 1], interval);
			auto rotation = lerp(skinTransformsLastUpdate[i], skinTransformsCurrentUpdate[i], interval);
			translation.storeAligned(jointData->translation);
			rotation.storeAligned(jointData->rotation);
			jointData++;
		}
		jointBufferCurrent->unlock();
	}

	m_mesh->build(
		context.getRenderContext(),
		worldRenderPass,
		lastWorldTransform,
		worldTransform,
		jointBufferLast,
		jointBufferCurrent,
		distance,
		getParameterCallback()
	);

	// Save last rendered transform so we can properly write velocities next frame.
	if ((worldRenderPass.getPassFlags() & world::IWorldRenderPass::Last) != 0)
	{
		m_jointBuffers[0] = jointBufferCurrent;
		m_jointBuffers[1] = jointBufferLast;
	}
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

	Quaternion tmp;
	m_skinTransforms[m_index][skinIndex * 2 + 0].storeAligned((float*)&tmp);

	outTransform = Transform(
		m_skinTransforms[m_index][skinIndex * 2 + 1],
		tmp
	);
	return true;
}

}
