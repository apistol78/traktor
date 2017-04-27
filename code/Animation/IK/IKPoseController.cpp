/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/IK/IKPoseController.h"
#include "Core/Math/Const.h"
#include "Core/Log/Log.h"
#include "Physics/PhysicsManager.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.IKPoseController", IKPoseController, IPoseController)

IKPoseController::IKPoseController(physics::PhysicsManager* physicsManager, IPoseController* poseController, uint32_t solverIterations)
:	m_physicsManager(physicsManager)
,	m_poseController(poseController)
,	m_solverIterations(solverIterations)
{
}

IKPoseController::~IKPoseController()
{
	destroy();
}

void IKPoseController::setIgnoreBody(physics::Body* ignoreBody)
{
	m_ignoreBody = ignoreBody;
}

void IKPoseController::destroy()
{
	m_poseController = 0;
	m_physicsManager = 0;
}

void IKPoseController::setTransform(const Transform& transform)
{
}

bool IKPoseController::evaluate(
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms,
	bool& outUpdateController
)
{
	//uint32_t jointCount = skeleton->getJointCount();

	//// Evaluate unaffected pose.
	//if (m_poseController)
	//{
	//	m_poseController->evaluate(
	//		deltaTime,
	//		worldTransform,
	//		skeleton,
	//		jointTransforms,
	//		outPoseTransforms,
	//		outUpdateController
	//	);

	//	// Ensure we've enough transforms.
	//	for (size_t i = outPoseTransforms.size(); i < jointTransforms.size(); ++i)
	//		outPoseTransforms.push_back(jointTransforms[i]);
	//}
	//else
	//	outPoseTransforms = jointTransforms;

	//// Calculate resting bone transforms.
	//AlignedVector< Transform > jointLocalTransforms;
	//calculateJointLocalTransforms(skeleton, jointLocalTransforms);

	//// Build node system.
	//AlignedVector< Vector4 > nodes;
	//std::vector< std::pair< int, int > > edges(jointCount);

	//for (uint32_t i = 0; i < jointCount; ++i)
	//{
	//	const Joint* joint = skeleton->getJoint(i);
	//	int base = int(nodes.size());
	//	if (joint->getParent() >= 0)
	//	{
	//		T_ASSERT (joint->getParent() < int(edges.size()));
	//		
	//		Vector4 s = outPoseTransforms[joint->getParent()].translation();
	//		Vector4 e = outPoseTransforms[i].translation();

	//		nodes.push_back(n);
	//		edges[i] = std::make_pair(
	//			edges[bone->getParent()].second,
	//			base
	//		);
	//	}
	//}

	//// Solve IK by iteratively solving each edge individually.
	//for (uint32_t i = 0; i < m_solverIterations; ++i)
	//{
	//	for (uint32_t j = 0; j < jointCount; ++j)
	//	{
	//		const Bone* bone = skeleton->getBone(j);

	//		Vector4& sp = nodes[edges[j].first];
	//		Vector4& ep = nodes[edges[j].second];

	//		// Constraint 1; keep length.
	//		{
	//			Vector4 d = ep - sp;
	//			Scalar ln = d.length();
	//			Scalar e = bone->getLength() - ln;
	//			if (abs(e) > FUZZY_EPSILON)
	//			{
	//				d /= ln;
	//				ep += e * d * Scalar(0.5f);
	//				sp -= e * d * Scalar(0.5f);
	//			}
	//		}

	//		// Constraint 2; keep cone angle.
	//		if (bone->getEnableLimits())
	//		{
	//			int parent = bone->getParent();
	//			if (parent >= 0)
	//			{
	//				const float limit = max< float >(bone->getConeLimit().x, bone->getConeLimit().y);

	//				Vector4 d1 = (ep - sp).normalized();
	//				Vector4 d2 = (outPoseTransforms[parent] * jointLocalTransforms[j].axisZ()).normalized();

	//				float phi = acosf(dot3(d1, d2));
	//				if (abs(phi) >= limit)
	//				{
	//					Vector4 t1 = cross(d1, d2);
	//					Vector4 p = cross(d2, t1).normalized();
	//					Vector4 d = (d2 + p * Scalar(sinf(limit))).normalized();
	//					ep = sp + d * (ep - sp).length();
	//				}
	//			}
	//		}
	//	}
	//}

	//// Update pose transforms from node system.
	//for (uint32_t i = 0; i < jointCount; ++i)
	//{
	//	const Vector4& sp = nodes[edges[i].first];
	//	const Vector4& ep = nodes[edges[i].second];

	//	Vector4 axisZ = (ep - sp).normalized();
	//	Vector4 axisY = cross(axisZ, outPoseTransforms[i].axisX()).normalized();
	//	Vector4 axisX = cross(axisY, axisZ).normalized();

	//	outPoseTransforms[i] = Transform(Matrix44(
	//		axisX,
	//		axisY,
	//		axisZ,
	//		sp
	//	));
	//}

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
}
