#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/RagDoll/RagDollPoseController.h"
#include "Animation/RagDoll/RagDollPoseControllerData.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Physics/BallJointDesc.h"
#include "Physics/Body.h"
#include "Physics/CapsuleShapeDesc.h"
#include "Physics/ConeTwistJointDesc.h"
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
	const AlignedVector< Transform >& jointTransforms,
	const AlignedVector< Velocity >& velocities,
	IPoseController* trackPoseController
)
{
	if (!physicsManager)
		return false;

	AlignedVector< Transform > limbTransforms = jointTransforms;

	// Evaluate initial poses with tracking controller.
	if (trackPoseController)
	{
		bool updateController = false;
		trackPoseController->evaluate(
			0.0f,
			worldTransform,
			skeleton,
			jointTransforms,
			limbTransforms,
			updateController
		);
	}

	const uint32_t jointCount = skeleton->getJointCount();

	// Create limb dynamic bodies.
	for (uint32_t i = 0; i < jointCount; ++i)
	{
		Joint* joint = skeleton->getJoint(i);
		T_ASSERT(joint);

		const int32_t parent = joint->getParent();
		if (parent < 0)
			continue;

		Vector4 start = limbTransforms[parent].translation();
		Vector4 end = limbTransforms[i].translation();
		Scalar length = (end - start).length();

		if (length < joint->getRadius() * 2.0f)
		{
			m_limbs.push_back(nullptr);
			continue;
		}

		Vector4 centerOfMass = (start + end) * Scalar(0.5f);

		physics::CapsuleShapeDesc shapeDesc;
		shapeDesc.setCollisionGroup(data->m_collisionGroup);
		shapeDesc.setCollisionMask(data->m_collisionMask);
		shapeDesc.setRadius(joint->getRadius());
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
		orthogonalFrame(axisZ, axisX, axisY);

		Transform limbTransform(Matrix44(
			axisX,
			axisY,
			axisZ,
			centerOfMass
		));

		limb->setTransform(worldTransform * limbTransform);
		if (!velocities.empty())
		{
			limb->setLinearVelocity(velocities[i].linear);
			limb->setAngularVelocity(velocities[i].angular);
		}

		m_limbs.push_back(limb);
		m_deltaTransforms.push_back(limbTransform.inverse() * limbTransforms[i]);
	}

	// for (uint32_t i = 0; i < jointCount; ++i)
	// {
	// 	Joint* joint = skeleton->getJoint(i);
	// 	T_ASSERT(joint);

	// 	Ref< physics::Joint > limbJoint;

	// 	const Vector4 anchor = limbTransforms[i].translation().xyz1();

	// 	int32_t parentIndex = joint->getParent();
	// 	if (parentIndex < 0 || !m_limbs[parentIndex])
	// 	{
	// 		if (!m_limbs[i])
	// 			continue;

	// 		if (data->m_fixateJoints)
	// 		{
	// 			Ref< physics::BallJointDesc > jointDesc = new physics::BallJointDesc();
	// 			jointDesc->setAnchor(anchor);

	// 			limbJoint = physicsManager->createJoint(
	// 				jointDesc,
	// 				worldTransform,
	// 				m_limbs[i],
	// 				nullptr
	// 			);
	// 		}
	// 	}
	// 	else
	// 	{
	// 		Ref< physics::BallJointDesc > jointDesc = new physics::BallJointDesc();
	// 		jointDesc->setAnchor(anchor);

	// 		limbJoint = physicsManager->createJoint(
	// 			jointDesc,
	// 			worldTransform,
	// 			m_limbs[parentIndex],
	// 			m_limbs[i]
	// 		);
	// 	}

	// 	m_joints.push_back(limbJoint);
	// }

	m_worldTransform = worldTransform;

	m_trackPoseController = trackPoseController;
	m_trackLinearTension = Scalar(data->m_trackLinearTension);
	m_trackAngularTension = Scalar(data->m_trackAngularTension);
	m_trackDuration = data->m_trackDuration;

	setEnable(data->m_enabled);
	return true;
}

void RagDollPoseController::destroy()
{
	safeDestroy(m_trackPoseController);

	for (auto joint : m_joints)
		safeDestroy(joint);
	for (auto limb : m_limbs)
		safeDestroy(limb);

	m_limbs.resize(0);
	m_joints.resize(0);
}

void RagDollPoseController::setTransform(const Transform& transform)
{
	// Calculate delta transform since last setTransform.
	Transform deltaTransform = transform * m_worldTransform.inverse();

	// Update all limbs with delta transform.
	for (auto limb : m_limbs)
	{
		if (limb)
		{
			Transform limbTransform = limb->getTransform();
			limb->setTransform(deltaTransform * limbTransform);
		}
	}

	// Update tracking pose controller.
	if (m_trackPoseController)
		m_trackPoseController->setTransform(transform);

	m_worldTransform = transform;
}

bool RagDollPoseController::evaluate(
	float deltaTime,
	const Transform& worldTransform,
	const Skeleton* skeleton,
	const AlignedVector< Transform >& jointTransforms,
	AlignedVector< Transform >& outPoseTransforms,
	bool& outUpdateController
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
				deltaTime,
				worldTransform,
				skeleton,
				jointTransforms,
				outPoseTransforms,
				outUpdateController
			);
	}

	const Transform worldTransformInv = worldTransform.inverse();
	const uint32_t jointCount = skeleton->getJointCount();

	outPoseTransforms.resize(jointCount);

	// Build pose transforms from limb bodies.
	for (uint32_t i = 0; i < jointCount; ++i)
	{
		Joint* joint = skeleton->getJoint(i);
		T_ASSERT(joint);

		if (!m_limbs[i])
			continue;

		// if (m_trackPoseController)
		// {
		// 	const Scalar c_maxTension(10.0f);

		// 	Transform trackT = worldTransform * outPoseTransforms[i];
		// 	Transform limbT = m_limbs[i]->getTransform() * Transform(halfBoneN);

		// 	{
		// 		Vector4 Tl = (limbT).translation().xyz1();
		// 		Vector4 Tt = (trackT).translation().xyz1();

		// 		Vector4 Fl = Tt - Tl;
		// 		Vector4 Vl = m_limbs[i]->getVelocityAt(Tl, false);

		// 		Scalar damping = Scalar(1.0f) - dot3(Fl, Vl);
		// 		Scalar tension = min(m_trackLinearTension * damping * Scalar(deltaTime), c_maxTension);

		// 		m_limbs[i]->addForceAt(
		// 			Tl,
		// 			Fl * tension,
		// 			false
		// 		);
		// 	}

		// 	{
		// 		Vector4 Tl = (limbT * boneP).translation().xyz1();
		// 		Vector4 Tt = (trackT * boneP).translation().xyz1();

		// 		Vector4 Fl = Tt - Tl;
		// 		Vector4 Vl = m_limbs[i]->getVelocityAt(Tl, false);

		// 		Scalar damping = Scalar(1.0f) - dot3(Fl, Vl);
		// 		Scalar tension = min(m_trackLinearTension * damping * Scalar(deltaTime), c_maxTension);

		// 		m_limbs[i]->addForceAt(
		// 			Tl,
		// 			Fl * tension,
		// 			false
		// 		);
		// 	}

		// 	if (abs(dot3(limbT.axisX(), trackT.axisX())) < 1.0f - FUZZY_EPSILON)
		// 	{
		// 		Vector4 vR = cross(limbT.axisX(), trackT.axisX());
		// 		Scalar lnR = vR.length();
		// 		if (lnR > FUZZY_EPSILON)
		// 		{
		// 			Scalar m = Scalar(1.0f) - abs(dot3(trackT.axisX(), limbT.axisX()));
		// 			m_limbs[i]->addTorque((m * vR * m_trackAngularTension * Scalar(deltaTime)) / lnR, false);
		// 		}
		// 	}
		// }

		outPoseTransforms[i] = worldTransformInv * m_limbs[i]->getTransform() * m_deltaTransforms[i];
	}

	m_worldTransform = worldTransform;

	outUpdateController = true;
	return true;
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

	}
}
