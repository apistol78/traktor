#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/IK/IKPoseController.h"
#include "Core/Math/Const.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.IKPoseController", IKPoseController, IPoseController)

IKPoseController::IKPoseController(IPoseController* poseController, uint32_t solverIterations)
:	m_poseController(poseController)
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
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms,
	bool& outUpdateController
)
{
	const uint32_t jointCount = skeleton->getJointCount();

	// Evaluate unaffected pose.
	if (m_poseController)
	{
		m_poseController->evaluate(
			deltaTime,
			worldTransform,
			skeleton,
			jointTransforms,
			outPoseTransforms,
			outUpdateController
		);

		// Ensure we've enough transforms.
		for (size_t i = outPoseTransforms.size(); i < jointTransforms.size(); ++i)
			outPoseTransforms.push_back(jointTransforms[i]);
	}
	else
		outPoseTransforms = jointTransforms;

	AlignedVector< Vector4 > nodes(jointCount);
	AlignedVector< Scalar > lengths(jointCount, Scalar(0.0f));

	// Calculate skeleton bone lengths.
	for (uint32_t i = 0; i < jointCount; ++i)
	{
		const Joint* joint = skeleton->getJoint(i);

		// Node position.
		nodes[i] = jointTransforms[i].translation();

		// Node to parent bone length.
		if (joint->getParent() >= 0)
		{
			Vector4 s = jointTransforms[joint->getParent()].translation();
			Vector4 e = jointTransforms[i].translation();
			lengths[i] = (e - s).length();
		}
	}

	// Solve IK by iteratively solving each constraint individually.
	for (uint32_t i = 0; i < m_solverIterations; ++i)
	{
		for (uint32_t j = 0; j < jointCount; ++j)
		{
			const Joint* joint = skeleton->getJoint(i);
			if (joint->getParent() < 0)
				continue;

			Vector4& s = nodes[joint->getParent()];
			Vector4& e = nodes[i];

			// Constraint 1; keep length.
			{
				Vector4 d = e - s;
				Scalar ln = d.length();
				Scalar err = lengths[i] - ln;
				if (abs(err) > FUZZY_EPSILON)
				{
					d /= ln;
					e += err * d * Scalar(0.5f);
					s -= err * d * Scalar(0.5f);
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
			const Vector4& s = nodes[joint->getParent()];
			const Vector4& e = nodes[i];

			Vector4 axisZ = (e - s).normalized();
			Vector4 axisY = cross(axisZ, outPoseTransforms[i].axisX()).normalized();
			Vector4 axisX = cross(axisY, axisZ).normalized();

			outPoseTransforms[i] = Transform(Matrix44(
				axisX,
				axisY,
				axisZ,
				s
			));
		}
		else
		{
			outPoseTransforms[i] = Transform(
				nodes[i],
				jointTransforms[i].rotation()
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
}
