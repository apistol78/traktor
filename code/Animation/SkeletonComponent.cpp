/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "Animation/Skeleton.h"
#include "Animation/SkeletonComponent.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/IPoseController.h"
#include "Animation/Joint.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/JobManager.h"
#include "World/Entity.h"

#define T_USE_UPDATE_JOBS

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.SkeletonComponent", SkeletonComponent, world::IEntityComponent)

SkeletonComponent::SkeletonComponent(
	const Transform& transform,
	const resource::Proxy< Skeleton >& skeleton,
	IPoseController* poseController
)
:	m_transform(transform)
,	m_skeleton(skeleton)
,	m_poseController(poseController)
{
	if (m_skeleton)
	{
		calculateJointTransforms(
			m_skeleton,
			m_jointTransforms
		);
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
}

void SkeletonComponent::setTransform(const Transform& transform)
{
	m_transform = transform;

	// Let pose controller know that entity has been manually repositioned.
	if (m_poseController)
		m_poseController->setTransform(transform);
}

Aabb3 SkeletonComponent::getBoundingBox() const
{
	synchronize();

	Aabb3 boundingBox;
	if (!m_poseTransforms.empty())
	{
		for (uint32_t i = 0; i < uint32_t(m_poseTransforms.size()); ++i)
		{
			const Joint* joint = m_skeleton->getJoint(i);
			const Scalar radius = joint->getRadius();
			boundingBox.contain(m_poseTransforms[i].translation().xyz1(), radius);
		}
	}

	return boundingBox;
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
				m_jointTransforms
			);

		m_poseTransforms.reserve(m_jointTransforms.size());
		m_skeleton.consume();
	}

#if defined(T_USE_UPDATE_JOBS)
	m_updatePoseControllerJob = JobManager::getInstance().add([=](){
		updatePoseController(update.alternateTime, update.deltaTime);
	});
#else
	updatePoseController(update.alternateTime, update.deltaTime);
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
	{
		m_skeleton->findAllChildren(index, [&](uint32_t child){
			m_poseTransforms[child] = Tdelta * m_poseTransforms[child];
		});
	}

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
			m_poseTransforms
		);
	}

	// Ensure we have same number of pose transforms as bones.
	const size_t skeletonJointCount = m_jointTransforms.size();
	for (size_t i = m_poseTransforms.size(); i < skeletonJointCount; ++i)
		m_poseTransforms.push_back(m_jointTransforms[i]);
}

}
