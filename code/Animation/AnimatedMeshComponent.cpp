/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/AnimatedMeshComponent.h"

#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonComponent.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/IRenderSystem.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

#include <cmath>

namespace traktor::animation
{
namespace
{

/*! Number of updates between pose evaluations, by distance to the view. Since the skin,
 * and thus also the ray tracing acceleration structure, is only rebuilt when the pose
 * has changed this reduce both CPU and GPU cost of distant characters. */
const struct
{
	float distance;
	int32_t maxLodRank;
	int32_t updatePeriod;
}
c_updatePeriods[] = {
	{ 20.0f, 8, 1 },
	{ 40.0f, 16, 2 },
	{ 60.0f, 32, 3 },
	{ 80.0f, 64, 4 },
	{ std::numeric_limits< float >::max(), std::numeric_limits< int32_t >::max(), 8 }
};

/*! Additional period multiplier when not visible in any view. We must still evaluate
 * since we might cast a shadow into the view, or be hit by a ray traced probe, but at
 * a lower rate than something actually on screen. */
const int32_t c_notVisibleUpdatePeriodScale = 2;

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimatedMeshComponent", AnimatedMeshComponent, mesh::SkinnedMeshComponent)

AnimatedMeshComponent::AnimatedMeshComponent(
	const Transform& transform,
	const resource::Proxy< mesh::SkinnedMesh >& mesh,
	render::IRenderSystem* renderSystem)
	: mesh::SkinnedMeshComponent(mesh, renderSystem)
	, m_index(0)
{
	const uint32_t skinJointCount = m_mesh->getJointCount();

	m_jointInverseTransforms.resize(skinJointCount, Transform::identity());
	m_poseTransforms[0].resize(skinJointCount, Transform::identity());
	m_poseTransforms[1].resize(skinJointCount, Transform::identity());
}

void AnimatedMeshComponent::setOwner(world::Entity* owner)
{
	mesh::MeshComponent::setOwner(owner);

	m_jointRemap.resize(0);
	m_skeletonComponent = nullptr;

	if (owner != nullptr)
	{
		m_skeletonComponent = owner->getComponent< SkeletonComponent >();
		if (m_skeletonComponent != nullptr && m_skeletonComponent->getSkeleton())
		{
			auto skeleton = m_skeletonComponent->getSkeleton();
			const auto& jointTransforms = m_skeletonComponent->getJointTransforms();

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

Aabb3 AnimatedMeshComponent::getBoundingBox() const
{
	return m_skeletonComponent != nullptr ? m_skeletonComponent->getBoundingBox() : m_mesh->getBoundingBox();
}

void AnimatedMeshComponent::update(const world::UpdateParams& update)
{
	// Always ensure skin arrays are same size as mesh joints.
	const size_t skinJointCount = m_mesh->getJointCount();
	m_jointInverseTransforms.resize(skinJointCount);
	m_poseTransforms[0].resize(skinJointCount, Transform::identity());
	m_poseTransforms[1].resize(skinJointCount, Transform::identity());

	// Calculate skinning transforms.
	if (m_skeletonComponent != nullptr && m_skeletonComponent->getSkeleton() && m_skeletonComponent->getRevision() != m_revision)
	{
		// Ensure all transforms are calculated.
		m_skeletonComponent->synchronize();

		const auto& jointTransforms = m_skeletonComponent->getJointTransforms();
		const auto& poseTransforms = m_skeletonComponent->getPoseTransforms();

		const bool firstPose = bool(m_revision < 0);

		// Step to the other slot before writing; m_index always refer to the most
		// recent pose and 1 - m_index to the pose of the previous update.
		m_index = 1 - m_index;

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

			// No previous pose to interpolate from on our first update; use the
			// same pose for both slots so we don't blend out from the bind pose.
			if (firstPose)
				m_poseTransforms[1 - m_index] = m_poseTransforms[m_index];
		}

		m_revision = m_skeletonComponent->getRevision();
		m_skinModified = true;
	}

	mesh::SkinnedMeshComponent::update(update);
}

void AnimatedMeshComponent::setupSkin(const world::WorldRenderView& worldRenderView, render::RenderContext* renderContext, int32_t lodRank)
{
	// Reset here; the base setupSkin sets it when we actually (re)build the skin, and the
	// inherited setupAccelerationStructure reads it to decide whether to build the BLAS.
	m_setupBuiltSkin = false;

	const Scalar interval(worldRenderView.getInterval());
	const Transform worldTransform = m_transform.get(interval);
	float distance = std::numeric_limits< float >::max();

	const bool isVisible = worldRenderView.isBoxVisible(
		getBoundingBox(),
		worldTransform,
		distance);

	m_lastWorldTransform[1] = m_lastWorldTransform[0];
	m_lastWorldTransform[0] = worldTransform;

	// Calculate update period based on distance and ranking.
	int32_t updatePeriod = 1;
	for (const auto& lod : c_updatePeriods)
	{
		if (distance <= lod.distance && lodRank < lod.maxLodRank)
		{
			updatePeriod = lod.updatePeriod;
			break;
		}
	}

	if (!isVisible)
		updatePeriod *= c_notVisibleUpdatePeriodScale;

	m_updatePeriod = (worldRenderView.getIndex() == 0) ? updatePeriod : std::min(m_updatePeriod, updatePeriod);

	if (m_skeletonComponent != nullptr)
		m_skeletonComponent->setUpdatePeriod(m_updatePeriod);

	const auto& poseTransformsPreviousUpdate = m_poseTransforms[1 - m_index];
	const auto& poseTransformsCurrentUpdate = m_poseTransforms[m_index];

	// When the pose is evaluated at a reduced rate the two pose slots span more than a
	// single update, thus the update interval no longer apply; snap to the latest pose.
	const Scalar poseInterval = (m_updatePeriod > 1) ? 1.0_simd : interval;

	// Rebuild skin when the pose has changed and something is actually going to draw us.
	// Being culled in the view is not enough to skip since a shadow or ray tracing pass
	// might still draw us; use the visibility accumulated by the passes of the previous
	// frame, which is one frame late but never leaves us permanently stale.
	if (worldRenderView.getIndex() == 0)
	{
		m_visibleLastFrame = m_visibleThisFrame;
		m_visibleThisFrame = false;
	}

	const bool relevant = isVisible || m_visibleLastFrame || m_rtwInstance != nullptr;
	const bool buildSkin = (m_skinModified && relevant) || m_firstSetup;
	if (buildSkin)
	{
		// Interpolate between updates to get current build skin transforms.
		if (poseTransformsCurrentUpdate.size() > 0)
		{
			mesh::SkinnedMesh::JointData* jointData = (mesh::SkinnedMesh::JointData*)m_jointBuffer->lock();
			for (uint32_t i = 0; i < poseTransformsCurrentUpdate.size(); ++i)
			{
				const Transform poseTransform = lerp(poseTransformsPreviousUpdate[i], poseTransformsCurrentUpdate[i], poseInterval);
				const Transform skinTransform = poseTransform * m_jointInverseTransforms[i];
				skinTransform.translation().storeAligned(jointData->translation);
				skinTransform.rotation().e.storeAligned(jointData->rotation);
				jointData++;
			}
			m_jointBuffer->unlock();
		}
		m_skinModified = false;

		mesh::SkinnedMeshComponent::setupSkin(worldRenderView, renderContext, lodRank);
	}
	else if (m_rtwInstance && !fuzzyEqual(m_lastWorldTransform[1], worldTransform))
	{
		// Update RT instance transform only.
		m_rtwInstance->setTransform(worldTransform);
	}

	// The previous skin buffer only hold the skin of the previous frame if we've
	// rebuilt the skin this frame; else both buffers refer to the same skin. On our
	// very first setup there is no previous skin at all. At a reduced rate the previous
	// skin is several frames old, which would overstate our velocities, so report no
	// skin motion at all; the world transform still contribute proper velocities.
	// Accumulate across views since setup is called once per view but build passes
	// of all views are executed afterwards.
	const bool skinBuilt = buildSkin && !m_firstSetup && m_updatePeriod <= 1;
	if (worldRenderView.getIndex() == 0)
		m_skinBuilt = skinBuilt;
	else
		m_skinBuilt |= skinBuilt;

	m_firstSetup = false;
	m_lastDistance = (worldRenderView.getIndex() == 0) ? distance : std::min(m_lastDistance, distance);
}

void AnimatedMeshComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	const bool supportTechnique = (m_mesh->supportTechnique(worldRenderPass.getTechnique()));
	if (!supportTechnique)
		return;

	const Scalar interval(worldRenderView.getInterval());
	const Transform worldTransform = m_transform.get(interval);
	float distance = 0.0f;

	const bool isVisible = worldRenderView.isBoxVisible(
		getBoundingBox(),
		worldTransform,
		distance);

	// Accumulate visibility of all passes, of all views, so next frame's setup know if
	// our skin need to be rebuilt even when culled in the view itself.
	m_visibleThisFrame |= isVisible;

	if (isVisible)
		m_mesh->build(
			context.getRenderContext(),
			worldRenderPass,
			m_lastWorldTransform[1],
			worldTransform,
			m_skinBuilt ? m_skinBuffer[1] : m_skinBuffer[0],
			m_skinBuffer[0],
			distance,
			getParameterCallback());
}

bool AnimatedMeshComponent::getSkinTransform(render::handle_t jointName, Transform& outTransform) const
{
	if (!m_skeletonComponent)
		return false;

	auto skeleton = m_skeletonComponent->getSkeleton();
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
