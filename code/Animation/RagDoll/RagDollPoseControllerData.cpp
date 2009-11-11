#include "Animation/RagDoll/RagDollPoseControllerData.h"
#include "Animation/RagDoll/RagDollPoseController.h"
#include "Animation/SkeletonUtils.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.animation.RagDollPoseControllerData", RagDollPoseControllerData, IPoseControllerData)

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

	Ref< RagDollPoseController > poseController = gc_new< RagDollPoseController >();
	if (!poseController->create(physicsManager, skeleton, worldTransform, boneTransforms, velocities, true))
		return 0;

	return poseController;
}

bool RagDollPoseControllerData::serialize(Serializer& s)
{
	return true;
}

	}
}
