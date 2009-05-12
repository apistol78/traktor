#include "Animation/RagDoll/RagDollPoseController.h"
#include "Animation/Skeleton.h"
#include "Animation/Bone.h"
#include "Physics/PhysicsManager.h"
#include "Physics/CapsuleShapeDesc.h"
#include "Physics/DynamicBodyDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/ConeTwistJointDesc.h"
#include "Physics/BallJointDesc.h"
#include "Physics/DynamicBody.h"
#include "Physics/Joint.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RagDollPoseController", RagDollPoseController, PoseController)

RagDollPoseController::~RagDollPoseController()
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

bool RagDollPoseController::create(
	physics::PhysicsManager* physicsManager,
	const Skeleton* skeleton,
	const Matrix44& worldTransform,
	const AlignedVector< Matrix44 >& boneTransforms,
	const AlignedVector< Velocity >& velocities,
	bool initiallyDisabled
)
{
	if (!physicsManager)
		return false;

	for (uint32_t i = 0; i < skeleton->getBoneCount(); ++i)
	{
		Bone* bone = skeleton->getBone(i);

		Vector4 centerOfMass = Vector4(0.0f, 0.0f, bone->getLength() * 0.5f, 1.0f);

		physics::CapsuleShapeDesc shapeDesc;
		shapeDesc.setRadius(bone->getRadius());
		shapeDesc.setLength(bone->getLength());

		physics::DynamicBodyDesc bodyDesc;
		bodyDesc.setShape(&shapeDesc);
		bodyDesc.setMass(0.1f);
		bodyDesc.setAutoDisable(true);
		bodyDesc.setDisabled(initiallyDisabled);
		bodyDesc.setLinearDamping(0.3f);
		bodyDesc.setAngularDamping(0.3f);

		Ref< physics::DynamicBody > limb = checked_type_cast< physics::DynamicBody* >(physicsManager->createBody(&bodyDesc));
		if (!limb)
			return false;

		limb->setTransform(translate(centerOfMass) * boneTransforms[i] * worldTransform);
		
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
		Bone* bone = skeleton->getBone(i);

		int parentIndex = bone->getParent();
		if (parentIndex < 0)
			continue;

		const Vector4 anchor = boneTransforms[i].translation();
		const Vector4 twistAxis = boneTransforms[i].axisZ();
		const Vector4 coneAxis = boneTransforms[i].axisX();

		Ref< physics::Joint > joint;

		if (bone->getEnableLimits())
		{
			Ref< physics::ConeTwistJointDesc > jointDesc = gc_new< physics::ConeTwistJointDesc >();
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
			Ref< physics::BallJointDesc > jointDesc = gc_new< physics::BallJointDesc >();
			jointDesc->setAnchor(anchor);

			joint = physicsManager->createJoint(
				jointDesc,
				worldTransform,
				m_limbs[parentIndex],
				m_limbs[i]
			);
		}

		if (!joint)
			return false;

		m_joints.push_back(joint);
	}

	return true;
}

void RagDollPoseController::evaluate(
	float deltaTime,
	const Matrix44& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Matrix44 >& boneTransforms,
	AlignedVector< Matrix44 >& outPoseTransforms,
	bool& outUpdateController
)
{
	T_ASSERT (boneTransforms.size() == m_limbs.size());

	Matrix44 worldTransformInv = worldTransform.inverseOrtho();

	uint32_t limbCount = uint32_t(m_limbs.size());
	
	outPoseTransforms.resize(limbCount);
	for (uint32_t i = 0; i < limbCount; ++i)
	{
		const Bone* bone = skeleton->getBone(i);
		T_ASSERT (bone);

		Vector4 centerOfMass = Vector4(0.0f, 0.0f, -bone->getLength() * 0.5f, 1.0f);

		outPoseTransforms[i] = translate(centerOfMass) * m_limbs[i]->getTransform() * worldTransformInv;
	}

	outUpdateController = true;
}

void RagDollPoseController::estimateVelocities(
	const Skeleton* skeleton,
	AlignedVector< Velocity >& outVelocities
)
{
}

void RagDollPoseController::enableAllLimbs()
{
	for (RefArray< physics::DynamicBody >::iterator i = m_limbs.begin(); i != m_limbs.end(); ++i)
		(*i)->setEnable(true);
}

	}
}
