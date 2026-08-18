/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/SkeletonComponent.h"

#include "Animation/IPoseController.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/JobManager.h"
#include "World/Entity.h"

#include <cmath>
#include <cstdlib>

//#define T_USE_UPDATE_JOBS

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.SkeletonComponent", SkeletonComponent, world::IEntityComponent)

SkeletonComponent::SkeletonComponent(
	const Transform& transform,
	const resource::Proxy< Skeleton >& skeleton,
	IPoseController* poseController)
	: m_transform(transform)
	, m_skeleton(skeleton)
	, m_poseController(poseController)
	, m_revision(0)
{
	// Randomize phase of interleaved updates so not every single skeleton in the
	// world evaluate its pose on the same update.
	m_updatePhase = std::rand() % c_maxUpdatePeriod;

	if (m_skeleton)
	{
		calculateJointTransforms(
			m_skeleton,
			m_jointTransforms);
		m_poseTransforms.reserve(m_jointTransforms.size());
		updatePoseController(0.0f, 0.0f);
	}
}

void SkeletonComponent::destroy()
{
	synchronize();
	safeDestroy(m_poseController);
}

void SkeletonComponent::setOwner(world::Entity* owner)
{
	if (m_poseController)
		m_poseController->setOwner(owner);
}

void SkeletonComponent::setTransform(const Transform& transform)
{
	m_transform = transform;

	// Let pose controller know that entity has been manually repositioned.
	if (m_poseController)
		m_poseController->setTransform(transform);
}

IPoseController* SkeletonComponent::getActivePoseController() const
{
	return m_poseController ? m_poseController->getActivePoseController() : nullptr;
}

void SkeletonComponent::getPoseControllersOf(const TypeInfo& type, RefArray< IPoseController >& outControllers) const
{
	if (m_poseController)
		m_poseController->getPoseControllersOf(type, outControllers);
}

Aabb3 SkeletonComponent::getBoundingBox() const
{
	const Scalar c_radius = 0.5_simd;

	synchronize();

	// Bounding box is derived from the current pose; recalculate only when the pose
	// has actually changed since this is queried several times per frame, once per
	// render pass which need to cull the owner entity.
	if (m_boundingBoxRevision != m_revision)
	{
		Aabb3 boundingBox;
		for (const auto& poseTransform : m_poseTransforms)
			boundingBox.contain(poseTransform.translation().xyz1(), c_radius);

		m_boundingBox = boundingBox;
		m_boundingBoxRevision = m_revision;
	}

	return m_boundingBox;
}

void SkeletonComponent::update(const world::UpdateParams& update)
{
	synchronize();

	// Calculate original bone transforms in object space.
	if (m_skeleton.changed())
	{
		m_jointTransforms.resize(0);
		m_poseTransforms.resize(0);

		if (m_skeleton)
			calculateJointTransforms(
				m_skeleton,
				m_jointTransforms);

		m_poseTransforms.reserve(m_jointTransforms.size());
		m_skeleton.consume();
		m_revision++;
	}

	// Accumulate time of all updates so a reduced rate evaluation still progress the
	// animation at the correct rate; else distant skeletons would animate in slow motion.
	m_deferredDeltaTime += update.deltaTime;

	// Evaluate pose at a reduced rate when distant; interleaved through a per instance
	// phase so all skeletons of the same period doesn't evaluate on the same update.
	// Counter wrap at c_maxUpdatePeriod, which every period is a divisor of in order to
	// keep the rate even across the wrap.
	const int32_t updatePeriod = m_updatePeriod;
	const int32_t updateCount = m_updateCount;
	m_updateCount = (m_updateCount + 1) % c_maxUpdatePeriod;
	if (updatePeriod > 1 && ((updateCount + m_updatePhase) % updatePeriod) != 0)
		return;

	const double deltaTime = m_deferredDeltaTime;
	m_deferredDeltaTime = 0.0;

#if defined(T_USE_UPDATE_JOBS)
	m_updatePoseControllerJob = JobManager::getInstance().add([=, this]() {
		updatePoseController(update.alternateTime, deltaTime);
	});
#else
	updatePoseController(update.alternateTime, deltaTime);
#endif
}

void SkeletonComponent::synchronize() const
{
#if defined(T_USE_UPDATE_JOBS)
	if (m_updatePoseControllerJob)
	{
		m_updatePoseControllerJob->wait();
		m_updatePoseControllerJob = nullptr;
	}
#endif
}

bool SkeletonComponent::getJointTransform(render::handle_t jointName, Transform& outTransform) const
{
	uint32_t index;
	if (!m_skeleton->findJoint(jointName, index))
		return false;

	if (index >= m_jointTransforms.size())
		return false;

	outTransform = m_jointTransforms[index];
	return true;
}

bool SkeletonComponent::getPoseTransform(render::handle_t jointName, Transform& outTransform) const
{
	uint32_t index;
	if (!m_skeleton->findJoint(jointName, index))
		return false;

	synchronize();

	if (index >= m_poseTransforms.size())
		return false;

	outTransform = m_poseTransforms[index];
	return true;
}

bool SkeletonComponent::setPoseTransform(render::handle_t jointName, const Transform& transform, bool inclusive)
{
	uint32_t index;
	if (!m_skeleton->findJoint(jointName, index))
		return false;

	synchronize();

	if (index >= m_jointTransforms.size())
		return false;

	if (m_poseTransforms.empty())
		m_poseTransforms = m_jointTransforms;

	m_poseTransforms[index] = transform; // Tdelta * m_poseTransforms[index];

	if (inclusive)
	{
		const Transform Tdelta = transform * m_jointTransforms[index].inverse();
		m_skeleton->findAllChildren(index, [&](uint32_t child) {
			m_poseTransforms[child] = Tdelta * m_jointTransforms[child];
		});
	}

	m_revision++;
	return true;
}

bool SkeletonComponent::concatenatePoseTransform(render::handle_t jointName, const Transform& transform, bool inclusive)
{
	uint32_t index;
	if (!m_skeleton->findJoint(jointName, index))
		return false;

	synchronize();

	if (index >= m_jointTransforms.size())
		return false;

	if (m_poseTransforms.empty())
		m_poseTransforms = m_jointTransforms;

	const Transform Tset = m_poseTransforms[index] * transform;
	const Transform Tdelta = Tset * m_poseTransforms[index].inverse();
	m_poseTransforms[index] = Tdelta * m_poseTransforms[index];

	if (inclusive)
		m_skeleton->findAllChildren(index, [&](uint32_t child) {
			m_poseTransforms[child] = Tdelta * m_poseTransforms[child];
		});

	m_revision++;
	return true;
}

void SkeletonComponent::updatePoseController(double time, double deltaTime)
{
	// Calculate pose transforms and skinning transforms.
	if (m_skeleton && m_poseController)
	{
		m_poseTransforms.resize(0);

		// Evaluate pose transforms in object space.
		m_poseController->evaluate(
			time,
			deltaTime,
			m_transform,
			m_skeleton,
			m_jointTransforms,
			m_poseTransforms);

		m_revision++;
	}

	// Ensure we have same number of pose transforms as bones.
	const size_t skeletonJointCount = m_jointTransforms.size();
	for (size_t i = m_poseTransforms.size(); i < skeletonJointCount; ++i)
		m_poseTransforms.push_back(m_jointTransforms[i]);
}

}
