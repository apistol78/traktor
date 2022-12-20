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
#include "Animation/SkeletonUtils.h"
#include "Animation/IK/IKPoseController.h"
#include "Core/Math/Const.h"
#include "Core/Log/Log.h"
#include "Physics/PhysicsManager.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.IKPoseController", IKPoseController, IPoseController)

IKPoseController::IKPoseController(
	physics::PhysicsManager* physicsManager,
	IPoseController* poseController,
	uint32_t solverIterations
)
:	m_physicsManager(physicsManager)
,	m_poseController(poseController)
,	m_solverIterations(solverIterations)
{
}

IKPoseController::~IKPoseController()
{
	destroy();
}

void IKPoseController::destroy()
{
	m_poseController = nullptr;
}

void IKPoseController::setTransform(const Transform& transform)
{
}

bool IKPoseController::evaluate(
	float time,
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms
)
{
	const uint32_t jointCount = skeleton->getJointCount();

	// Evaluate unaffected pose.
	if (m_poseController)
	{
		m_poseController->evaluate(
			time,
			deltaTime,
			worldTransform,
			skeleton,
			jointTransforms,
			outPoseTransforms
		);

		// Ensure we've enough transforms.
		for (size_t i = outPoseTransforms.size(); i < jointTransforms.size(); ++i)
			outPoseTransforms.push_back(jointTransforms[i]);
	}
	else
		outPoseTransforms = jointTransforms;

	AlignedVector< Vector4 > nodes(jointCount);
	AlignedVector< Scalar > lengths(jointCount, 0.0_simd);

	// Calculate skeleton bone lengths.
	for (uint32_t i = 0; i < jointCount; ++i)
	{
		// Node position.
		nodes[i] = outPoseTransforms[i].translation().xyz1();

		// Node to parent bone length.
		const Joint* joint = skeleton->getJoint(i);
		if (joint->getParent() >= 0)
		{
			const Vector4 s = jointTransforms[joint->getParent()].translation();
			const Vector4 e = jointTransforms[i].translation();
			lengths[i] = (e - s).length();
		}
	}

	// Solve IK by iteratively solving each constraint individually.
	for (uint32_t i = 0; i < m_solverIterations; ++i)
	{
		// Constraint 2; always above ground.
		for (uint32_t j = 0; j < jointCount; ++j)
		{
			const Vector4 n = worldTransform * nodes[j];

			// Find intersection above node position.
			const physics::QueryFilter filter(~0U);
			physics::QueryResult result;
			if (!m_physicsManager->queryRay(n, Vector4(0.0f, 1.0f, 0.0f, 0.0f), 4.0f, filter, false, result))
				continue;

			// Move node to intersection.
			nodes[j] = worldTransform.inverse() * (result.position + Vector4(0.0f, 0.5f, 0.0f, 0.0f)).xyz1();
		}

		// Constraint 1; keep length.
		for (uint32_t j = 0; j < jointCount; ++j)
		{
			const Joint* joint = skeleton->getJoint(j);
			if (joint->getParent() < 0)
				continue;

			Vector4& s = nodes[joint->getParent()];
			Vector4& e = nodes[j];

			{
				Vector4 d = e - s;
				Scalar ln = d.length();
				Scalar err = lengths[j] - ln;
				if (abs(err) > FUZZY_EPSILON)
				{
					d /= ln;
					e += err * d * 0.5_simd;
					s -= err * d * 0.5_simd;
				}
			}
		}
	}

	// Update pose transforms from node system.
	for (uint32_t i = 0; i < jointCount; ++i)
	{
		const Joint* joint = skeleton->getJoint(i);
		if (joint->getParent() >= 0)
		{
			const Vector4& sref = outPoseTransforms[joint->getParent()].translation();
			const Vector4& eref = outPoseTransforms[i].translation();

			const Vector4 axisZref = (eref - sref).normalized();

			const Vector4& sik = nodes[joint->getParent()];
			const Vector4& eik = nodes[i];

			Vector4 axisZik = (eik - sik).normalized();
			Vector4 axisXik, axisYik;
			orthogonalFrame(axisZik, axisYik, axisXik);

			const Quaternion Qr(
				axisZref,
				axisZik
			);
			const Quaternion Qrr = outPoseTransforms[i].rotation() * Qr;

			outPoseTransforms[i] = Transform(eik, Qrr);
		}
		else
		{
			outPoseTransforms[i] = Transform(
				nodes[i].xyz0(),
				outPoseTransforms[i].rotation()
			);
		}
	}

	return true;
}

void IKPoseController::estimateVelocities(
	const Skeleton* skeleton,
	AlignedVector< Velocity >& outVelocities
)
{
	// Estimate velocities without IK.
	if (m_poseController)
		m_poseController->estimateVelocities(
			skeleton,
			outVelocities
		);
}

}
