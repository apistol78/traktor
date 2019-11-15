#include "Animation/SkeletonUtils.h"
#include "Animation/RagDoll/RagDollPoseController.h"
#include "Animation/RagDoll/RagDollPoseControllerData.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Physics/CollisionSpecification.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{

const resource::Id< physics::CollisionSpecification > c_defaultCollision(Guid(L"{F9805131-50C2-504C-9421-13C99E44616C}"));
const resource::Id< physics::CollisionSpecification > c_interactableCollision(Guid(L"{09CB1141-1924-3349-934A-CEB9728D7A61}"));

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RagDollPoseControllerData", 5, RagDollPoseControllerData, IPoseControllerData)

RagDollPoseControllerData::RagDollPoseControllerData()
:	m_autoDeactivate(true)
,	m_enabled(true)
,	m_fixateJoints(false)
,	m_limbMass(1.0f)
,	m_linearDamping(0.1f)
,	m_angularDamping(0.1f)
,	m_linearThreshold(0.8f)
,	m_angularThreshold(1.0f)
,	m_trackLinearTension(0.0f)
,	m_trackAngularTension(0.0f)
,	m_trackDuration(0.0f)
{
	m_collisionGroup.insert(c_defaultCollision);
	m_collisionMask.insert(c_defaultCollision);
	m_collisionMask.insert(c_interactableCollision);
}

Ref< IPoseController > RagDollPoseControllerData::createInstance(
	resource::IResourceManager* resourceManager,
	physics::PhysicsManager* physicsManager,
	const Skeleton* skeleton,
	const Transform& worldTransform
) const
{
	Ref< IPoseController > trackPoseController;

	AlignedVector< Transform > jointTransforms;
	calculateJointTransforms(skeleton, jointTransforms);

	AlignedVector< IPoseController::Velocity > velocities;
	velocities.reserve(jointTransforms.size());

	if (m_trackPoseController)
	{
		trackPoseController = m_trackPoseController->createInstance(
			resourceManager,
			physicsManager,
			skeleton,
			worldTransform
		);
		if (!trackPoseController)
			return nullptr;

		trackPoseController->estimateVelocities(skeleton, velocities);
	}

	for (uint32_t i = (uint32_t)velocities.size(); i < (uint32_t)jointTransforms.size(); ++i)
	{
		IPoseController::Velocity velocity;
		velocity.linear =
		velocity.angular = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		velocities.push_back(velocity);
	}

	Ref< RagDollPoseController > poseController = new RagDollPoseController();
	if (!poseController->create(
		resourceManager,
		physicsManager,
		this,
		skeleton,
		worldTransform,
		jointTransforms,
		velocities,
		trackPoseController
	))
		return nullptr;

	return poseController;
}

void RagDollPoseControllerData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< RagDollPoseControllerData >() >= 5);

	s >> MemberStlSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"collisionGroup", m_collisionGroup);
	s >> MemberStlSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"collisionMask", m_collisionMask);
	s >> Member< bool >(L"autoDeactivate", m_autoDeactivate);
	s >> Member< bool >(L"enabled", m_enabled);
	s >> Member< bool >(L"fixateJoints", m_fixateJoints);
	s >> Member< float >(L"limbMass", m_limbMass, AttributeUnit(AuKilograms));
	s >> Member< float >(L"linearDamping", m_linearDamping, AttributeUnit(AuPercent));
	s >> Member< float >(L"angularDamping", m_angularDamping, AttributeUnit(AuPercent));
	s >> Member< float >(L"linearThreshold", m_linearThreshold, AttributeUnit(AuMetres, true));
	s >> Member< float >(L"angularThreshold", m_angularThreshold, AttributeUnit(AuMetres, true));
	s >> MemberRef< const IPoseControllerData >(L"trackPoseController", m_trackPoseController);
	s >> Member< float >(L"trackLinearTension", m_trackLinearTension);
	s >> Member< float >(L"trackAngularTension", m_trackAngularTension);
	s >> Member< float >(L"trackDuration", m_trackDuration, AttributeUnit(AuSeconds));
}

	}
}
