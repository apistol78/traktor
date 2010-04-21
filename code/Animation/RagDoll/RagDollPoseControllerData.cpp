#include "Core/Serialization/ISerializer.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/RagDoll/RagDollPoseController.h"
#include "Animation/RagDoll/RagDollPoseControllerData.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.RagDollPoseControllerData", 1, RagDollPoseControllerData, IPoseControllerData)

RagDollPoseControllerData::RagDollPoseControllerData()
:	m_collisionGroup(~0UL)
,	m_autoDeactivate(true)
,	m_enabled(true)
,	m_fixateBones(false)
,	m_limbMass(1.0f)
,	m_linearDamping(0.1f)
,	m_angularDamping(0.1f)
{
}

Ref< IPoseController > RagDollPoseControllerData::createInstance(
	resource::IResourceManager* resourceManager,
	physics::PhysicsManager* physicsManager,
	const Skeleton* skeleton,
	const Transform& worldTransform
)
{
	AlignedVector< Transform > boneTransforms;
	calculateBoneTransforms(skeleton, boneTransforms);

	AlignedVector< IPoseController::Velocity > velocities;
	for (uint32_t i = 0; i < uint32_t(boneTransforms.size()); ++i)
	{
		IPoseController::Velocity velocity;
		velocity.linear =
		velocity.angular = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		velocities.push_back(velocity);
	}

	Ref< RagDollPoseController > poseController = new RagDollPoseController();
	if (!poseController->create(
		physicsManager,
		skeleton,
		worldTransform,
		boneTransforms,
		velocities,
		m_collisionGroup,
		m_autoDeactivate,
		m_enabled,
		m_fixateBones,
		m_limbMass,
		m_linearDamping,
		m_angularDamping
	))
		return 0;

	return poseController;
}

bool RagDollPoseControllerData::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
	{
		s >> Member< uint32_t >(L"collisionGroup", m_collisionGroup);
		s >> Member< bool >(L"autoDeactivate", m_autoDeactivate);
		s >> Member< bool >(L"enabled", m_enabled);
		s >> Member< bool >(L"fixateBones", m_fixateBones);
		s >> Member< float >(L"limbMass", m_limbMass);
		s >> Member< float >(L"linearDamping", m_linearDamping);
		s >> Member< float >(L"angularDamping", m_angularDamping);
	}
	return true;
}

	}
}
