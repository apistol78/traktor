#include "Animation/Bone.h"
#include "Animation/Skeleton.h"
#include "Animation/RagDoll/RagDollPoseController.h"
#include "Core/Math/Const.h"
#include "Physics/BallJointDesc.h"
#include "Physics/CapsuleShapeDesc.h"
#include "Physics/ConeTwistJointDesc.h"
#include "Physics/DynamicBody.h"
#include "Physics/DynamicBodyDesc.h"
#include "Physics/Joint.h"
#include "Physics/PhysicsManager.h"
#include "Physics/StaticBodyDesc.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RagDollPoseController", RagDollPoseController, IPoseController)

RagDollPoseController::RagDollPoseController()
:	m_enable(false)
{
}

RagDollPoseController::~RagDollPoseController()
{
	destroy();
}

bool RagDollPoseController::create(
	physics::PhysicsManager* physicsManager,
	const Skeleton* skeleton,
	const Transform& worldTransform,
	const AlignedVector< Transform >& boneTransforms,
	const AlignedVector< Velocity >& velocities,
	uint32_t collisionGroup,
	bool autoDeactivate,
	bool enabled,
	bool fixateBones,
	float limbMass,
	float linearDamping,
	float angularDamping
)
{
	if (!physicsManager)
		return false;

	for (uint32_t i = 0; i < skeleton->getBoneCount(); ++i)
	{
		Bone* bone = skeleton->getBone(i);

		Vector4 centerOfMass = Vector4(0.0f, 0.0f, bone->getLength() * 0.5f, 1.0f);

		physics::CapsuleShapeDesc shapeDesc;
		shapeDesc.setGroup(collisionGroup);
		shapeDesc.setRadius(bone->getRadius());
		shapeDesc.setLength(bone->getLength());

		physics::DynamicBodyDesc bodyDesc;
		bodyDesc.setShape(&shapeDesc);
		bodyDesc.setMass(limbMass);
		bodyDesc.setAutoDeactivate(autoDeactivate);
		bodyDesc.setActive(!autoDeactivate);
		bodyDesc.setLinearDamping(linearDamping);
		bodyDesc.setAngularDamping(angularDamping);

		Ref< physics::DynamicBody > limb = checked_type_cast< physics::DynamicBody* >(physicsManager->createBody(&bodyDesc));
		if (!limb)
			return false;

		limb->setTransform(worldTransform * boneTransforms[i] * Transform(centerOfMass));
		
		// Set initial velocities.
		if (!velocities.empty())
		{
			limb->setLinearVelocity(velocities[i].linear);
			limb->setAngularVelocity(velocities[i].angular);
		}

		m_limbs.push_back(limb);
	}

	for (uint32_t i = 0; i < skeleton->getBoneCount(); ++i)
	{
		Ref< physics::Joint > joint;

		Bone* bone = skeleton->getBone(i);
		T_ASSERT (bone);

		const Vector4 anchor = boneTransforms[i].translation().xyz1();
		const Vector4 twistAxis = boneTransforms[i].axisZ();
		const Vector4 coneAxis = boneTransforms[i].axisX();

		int parentIndex = bone->getParent();
		if (parentIndex < 0)
		{
			if (fixateBones)
			{
				if (bone->getEnableLimits())
				{
					Ref< physics::ConeTwistJointDesc > jointDesc = new physics::ConeTwistJointDesc();
					jointDesc->setAnchor(anchor);
					jointDesc->setConeAxis(coneAxis);
					jointDesc->setTwistAxis(twistAxis);
					jointDesc->setConeAngles(bone->getConeLimit().x, bone->getConeLimit().y);
					jointDesc->setTwistAngle(bone->getTwistLimit());

					joint = physicsManager->createJoint(
						jointDesc,
						worldTransform,
						m_limbs[i],
						0
					);
				}
				else
				{
					Ref< physics::BallJointDesc > jointDesc = new physics::BallJointDesc();
					jointDesc->setAnchor(anchor);

					joint = physicsManager->createJoint(
						jointDesc,
						worldTransform,
						m_limbs[i],
						0
					);
				}
			}
		}
		else
		{
			if (bone->getEnableLimits())
			{
				Ref< physics::ConeTwistJointDesc > jointDesc = new physics::ConeTwistJointDesc();
				jointDesc->setAnchor(anchor);
				jointDesc->setConeAxis(coneAxis);
				jointDesc->setTwistAxis(twistAxis);
				jointDesc->setConeAngles(bone->getConeLimit().x, bone->getConeLimit().y);
				jointDesc->setTwistAngle(bone->getTwistLimit());

				joint = physicsManager->createJoint(
					jointDesc,
					worldTransform,
					m_limbs[parentIndex],
					m_limbs[i]
				);
			}
			else
			{
				Ref< physics::BallJointDesc > jointDesc = new physics::BallJointDesc();
				jointDesc->setAnchor(anchor);

				joint = physicsManager->createJoint(
					jointDesc,
					worldTransform,
					m_limbs[parentIndex],
					m_limbs[i]
				);
			}
		}

		if (!joint)
			return false;

		m_joints.push_back(joint);
	}

	m_worldTransform = worldTransform;

	setEnable(enabled);

	return true;
}

void RagDollPoseController::destroy()
{
	for (RefArray< physics::Joint >::iterator i = m_joints.begin(); i != m_joints.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}

	for (RefArray< physics::DynamicBody >::iterator j = m_limbs.begin(); j != m_limbs.end(); ++j)
	{
		if (*j)
			(*j)->destroy();
	}

	m_limbs.resize(0);
	m_joints.resize(0);
}

void RagDollPoseController::setTransform(const Transform& transform)
{
	// Calculate delta transform since last setTransform.
	Transform deltaTransform = m_worldTransform.inverse() * transform;

	// Update all limbs with delta transform.
	for (RefArray< physics::DynamicBody >::iterator i = m_limbs.begin(); i != m_limbs.end(); ++i)
	{
		Transform limbTransform = (*i)->getTransform();
		(*i)->setTransform(deltaTransform * limbTransform);
	}
}

void RagDollPoseController::evaluate(
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& boneTransforms,
	AlignedVector< Transform >& outPoseTransforms,
	bool& outUpdateController
)
{
	T_ASSERT (boneTransforms.size() == m_limbs.size());

	Transform worldTransformInv = worldTransform.inverse();
	uint32_t limbCount = uint32_t(m_limbs.size());
	
	outPoseTransforms.resize(limbCount);
	for (uint32_t i = 0; i < limbCount; ++i)
	{
		const Bone* bone = skeleton->getBone(i);
		T_ASSERT (bone);

		Vector4 centerOfMass = Vector4(0.0f, 0.0f, -bone->getLength() * 0.5f, 1.0f);
		outPoseTransforms[i] = worldTransformInv * m_limbs[i]->getTransform() * Transform(centerOfMass);
	}

	outUpdateController = true;
}

void RagDollPoseController::estimateVelocities(
	const Skeleton* skeleton,
	AlignedVector< Velocity >& outVelocities
)
{
}

void RagDollPoseController::setEnable(bool enable)
{
	if (enable == m_enable)
		return;

	if (enable)
	{
		for (RefArray< physics::DynamicBody >::iterator i = m_limbs.begin(); i != m_limbs.end(); ++i)
			(*i)->setEnable(true);
		for (RefArray< physics::Joint >::iterator i = m_joints.begin(); i != m_joints.end(); ++i)
			(*i)->setEnable(true);
	}
	else
	{
		for (RefArray< physics::Joint >::iterator i = m_joints.begin(); i != m_joints.end(); ++i)
			(*i)->setEnable(false);
		for (RefArray< physics::DynamicBody >::iterator i = m_limbs.begin(); i != m_limbs.end(); ++i)
			(*i)->setEnable(false);
	}

	m_enable = enable;
}

bool RagDollPoseController::isEnable() const
{
	return m_enable;
}

const RefArray< physics::DynamicBody >& RagDollPoseController::getLimbs() const
{
	return m_limbs;
}

	}
}
