/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/RagDoll/RagDollPoseController.h"
#include "Animation/RagDoll/RagDollPoseControllerData.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Physics/BallJointDesc.h"
#include "Physics/Body.h"
#include "Physics/CapsuleShapeDesc.h"
#include "Physics/ConeTwistJointDesc.h"
#include "Physics/DofJointDesc.h"
#include "Physics/DynamicBodyDesc.h"
#include "Physics/HingeJointDesc.h"
#include "Physics/Joint.h"
#include "Physics/PhysicsManager.h"
#include "Physics/StaticBodyDesc.h"

namespace traktor::animation
{
	namespace
	{
		
uint32_t s_clusterId = 10000;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RagDollPoseController", RagDollPoseController, IPoseController)

RagDollPoseController::RagDollPoseController()
:	m_trackDuration(0.0f)
,	m_enable(false)
{
}

RagDollPoseController::~RagDollPoseController()
{
	destroy();
}

bool RagDollPoseController::create(
	resource::IResourceManager* resourceManager,
	physics::PhysicsManager* physicsManager,
	const RagDollPoseControllerData* data,
	const Skeleton* skeleton,
	const Transform& worldTransform,
	const AlignedVector< Transform >& jointTransformsImmutable,
	IPoseController* trackPoseController
)
{
	if (!physicsManager)
		return false;

	AlignedVector< Transform > jointTransforms = jointTransformsImmutable;

	// Evaluate initial poses with tracking controller.
	if (trackPoseController)
	{
		trackPoseController->evaluate(
			0.0f,
			0.0f,
			worldTransform,
			skeleton,
			jointTransformsImmutable,
			jointTransforms
		);
	}

	const uint32_t jointCount = skeleton->getJointCount();

	// Create limb dynamic bodies.
	m_limbs.resize(jointCount);
	for (uint32_t i = 0; i < jointCount; ++i)
	{
		Joint* joint = skeleton->getJoint(i);
		T_ASSERT(joint);

		const int32_t parent = joint->getParent();
		if (parent < 0)
		{
			m_limbs[i] = nullptr;
			continue;
		}

		const Vector4 start = jointTransforms[parent].translation();
		const Vector4 end = jointTransforms[i].translation();

		float length = (end - start).length();
		float radius = joint->getRadius();
		if (radius > length / 2.0f)
			radius = length / 2.0f;

		const Vector4 centerOfMass = (start + end) * 0.5_simd;

		physics::CapsuleShapeDesc shapeDesc;
		shapeDesc.setCollisionGroup(data->m_collisionGroup);
		shapeDesc.setCollisionMask(data->m_collisionMask);
		shapeDesc.setRadius(radius);
		shapeDesc.setLength(length);

		physics::DynamicBodyDesc bodyDesc;
		bodyDesc.setShape(&shapeDesc);
		bodyDesc.setMass(data->m_limbMass);
		bodyDesc.setAutoDeactivate(data->m_autoDeactivate);
		bodyDesc.setActive(true);
		bodyDesc.setLinearDamping(data->m_linearDamping);
		bodyDesc.setAngularDamping(data->m_angularDamping);
		bodyDesc.setLinearThreshold(data->m_linearThreshold);
		bodyDesc.setAngularThreshold(data->m_angularThreshold);

		Ref< physics::Body > limb = physicsManager->createBody(resourceManager, &bodyDesc, T_FILE_LINE_W);
		if (!limb)
			return false;

		Vector4 axisZ = (end - start).normalized();
		Vector4 axisX, axisY;
		orthogonalFrame(axisZ, axisY, axisX);

		const Matrix44 m1(axisX, axisY, axisZ, Vector4::origo());
		const Matrix44 m2 = translate(centerOfMass);
		const Transform limbTransform(m2 * m1);

		limb->setTransform(worldTransform * limbTransform);
		limb->setClusterId(s_clusterId);

		m_limbs[i] = limb;
	}
	T_FATAL_ASSERT(m_limbs.size() == jointCount);

	// Associate limbs of joints.
	m_deltaLimbs.resize(jointCount);
	for (uint32_t i = 0; i < jointCount; ++i)
	{
		Joint* joint = skeleton->getJoint(i);
		T_ASSERT(joint);

		const int32_t parent = joint->getParent();
		if (parent >= 0)
		{
			m_deltaLimbs[i] = m_limbs[i];
			if (m_deltaLimbs[parent] == nullptr)
				m_deltaLimbs[parent] = m_limbs[i];
		}
	}

	// Calculate delta transforms.
	m_deltaTransforms.resize(jointCount);
	for (uint32_t i = 0; i < jointCount; ++i)
	{
		Transform limbTransform = worldTransform.inverse() * m_deltaLimbs[i]->getTransform();
		m_deltaTransforms[i] = limbTransform.inverse() * jointTransforms[i];
	}

	// Create joint constraints.
	for (uint32_t i = 0; i < jointCount; ++i)
	{
		Joint* joint = skeleton->getJoint(i);
		T_ASSERT(joint);

		const Vector4 anchor = jointTransforms[i].translation().xyz1();

		skeleton->findChildren(i, [&](uint32_t child) {
			if (m_deltaLimbs[child] == m_deltaLimbs[i])
				return;

			Ref< physics::Joint > limbJoint;
			if (data->m_constraintAxises == 0)
			{
				Ref< physics::DofJointDesc > jointDesc = new physics::DofJointDesc();
				limbJoint = physicsManager->createJoint(
					jointDesc,
					worldTransform,
					m_deltaLimbs[i],
					m_deltaLimbs[child]
				);
			}
			else if (data->m_constraintAxises == (1 | 2 | 4))
			{
				Ref< physics::BallJointDesc > jointDesc = new physics::BallJointDesc();
				jointDesc->setAnchor(anchor);
				limbJoint = physicsManager->createJoint(
					jointDesc,
					worldTransform,
					m_deltaLimbs[i],
					m_deltaLimbs[child]
				);
			}
			else
			{
				Ref< physics::HingeJointDesc > jointDesc = new physics::HingeJointDesc();
				jointDesc->setAnchor(anchor);

				if (data->m_constraintAxises == 1)
					jointDesc->setAxis(jointTransforms[i].axisX());
				else if (data->m_constraintAxises == 2)
					jointDesc->setAxis(jointTransforms[i].axisY());
				else if (data->m_constraintAxises == 4)
					jointDesc->setAxis(jointTransforms[i].axisZ());
				else
					return;

				limbJoint = physicsManager->createJoint(
					jointDesc,
					worldTransform,
					m_deltaLimbs[i],
					m_deltaLimbs[child]
				);
			}

			if (!limbJoint)
				return;

			m_joints.push_back(limbJoint);
		});
	}

	m_worldTransform = worldTransform;

	m_trackPoseController = trackPoseController;
	m_trackLinearTension = Scalar(data->m_trackLinearTension);
	m_trackAngularTension = Scalar(data->m_trackAngularTension);
	m_trackDuration = data->m_trackDuration;

	setEnable(data->m_enabled);

	s_clusterId++;
	return true;
}

void RagDollPoseController::destroy()
{
	safeDestroy(m_trackPoseController);
	safeDestroy(m_joints);
	safeDestroy(m_limbs);

	m_deltaLimbs.resize(0);
	m_deltaTransforms.resize(0);
}

void RagDollPoseController::setTransform(const Transform& transform)
{
	// Calculate delta transform since last setTransform.
	const Transform deltaTransform = transform * m_worldTransform.inverse();

	// Update all limbs with delta transform.
	for (auto limb : m_limbs)
	{
		if (limb)
		{
			const Transform limbTransform = limb->getTransform();
			limb->setTransform(deltaTransform * limbTransform);
		}
	}

	// Update tracking pose controller.
	if (m_trackPoseController)
		m_trackPoseController->setTransform(transform);

	m_worldTransform = transform;
}

bool RagDollPoseController::evaluate(
	float time,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms
)
{
	T_ASSERT(jointTransforms.size() == m_limbs.size());

	// Update tracking pose controller.
	if (m_trackPoseController)
	{
		if (m_trackDuration > 0.0f)
		{
			m_trackDuration -= deltaTime;
			if (m_trackDuration <= 0.0f)
				m_trackPoseController = nullptr;
		}
		if (m_trackPoseController)
			m_trackPoseController->evaluate(
				time,
				deltaTime,
				worldTransform,
				skeleton,
				jointTransforms,
				outPoseTransforms
			);
	}

	const Transform worldTransformInv = worldTransform.inverse();
	const uint32_t jointCount = skeleton->getJointCount();

	// Build pose transforms from limb bodies.
	outPoseTransforms.resize(jointCount);
	for (uint32_t i = 0; i < jointCount; ++i)
	{
		const Transform limbTransform = worldTransformInv * m_deltaLimbs[i]->getTransform();
		outPoseTransforms[i] = limbTransform * m_deltaTransforms[i];
	}

	m_worldTransform = worldTransform;
	return true;
}

void RagDollPoseController::setEnable(bool enable)
{
	if (enable == m_enable)
		return;

	for (auto limb : m_limbs)
	{
		if (limb)
			limb->setEnable(enable);
	}

	m_enable = enable;
}

bool RagDollPoseController::isEnable() const
{
	return m_enable;
}

const RefArray< physics::Body >& RagDollPoseController::getLimbs() const
{
	return m_limbs;
}

const RefArray< physics::Joint >& RagDollPoseController::getJoints() const
{
	return m_joints;
}

}
