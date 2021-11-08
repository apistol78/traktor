#include "Animation/AnimationClassFactory.h"
#include "Animation/AnimatedMeshComponent.h"
#include "Animation/Animation/StatePoseController.h"
#include "Animation/Boids/BoidsComponent.h"
#include "Animation/Cloth/ClothComponent.h"
#include "Animation/IK/IKPoseController.h"
#include "Animation/PathEntity/PathComponent.h"
#include "Animation/PathEntity/PathComponentData.h"
#include "Animation/RagDoll/RagDollPoseController.h"
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Class/Boxes/BoxedTransform.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Physics/Body.h"
#include "Physics/PhysicsManager.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{

Transform AnimatedMeshComponent_getJointTransform(AnimatedMeshComponent* self, const std::wstring& jointName)
{
	Transform transform;
	self->getJointTransform(
		render::getParameterHandle(jointName),
		transform
	);
	return transform;
}

Transform AnimatedMeshComponent_getPoseTransform(AnimatedMeshComponent* self, const std::wstring& jointName)
{
	Transform transform;
	self->getPoseTransform(
		render::getParameterHandle(jointName),
		transform
	);
	return transform;
}

Transform AnimatedMeshComponent_getSkinTransform(AnimatedMeshComponent* self, const std::wstring& jointName)
{
	Transform transform;
	self->getSkinTransform(
		render::getParameterHandle(jointName),
		transform
	);
	return transform;
}

void AnimatedMeshComponent_setPoseTransform(AnimatedMeshComponent* self, const std::wstring& jointName, const Transform& transform, bool inclusive)
{
	self->setPoseTransform(
		render::getParameterHandle(jointName),
		transform,
		inclusive
	);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationClassFactory", 0, AnimationClassFactory, IRuntimeClassFactory)

void AnimationClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classAnimatedMeshComponent = new AutoRuntimeClass< AnimatedMeshComponent >();
	classAnimatedMeshComponent->addMethod("getJointTransform", &AnimatedMeshComponent_getJointTransform);
	classAnimatedMeshComponent->addMethod("getPoseTransform", &AnimatedMeshComponent_getPoseTransform);
	classAnimatedMeshComponent->addMethod("getSkinTransform", &AnimatedMeshComponent_getSkinTransform);
	classAnimatedMeshComponent->addMethod("setPoseTransform", &AnimatedMeshComponent_setPoseTransform);
	classAnimatedMeshComponent->addMethod("setPoseController", &AnimatedMeshComponent::setPoseController);
	classAnimatedMeshComponent->addMethod("getPoseController", &AnimatedMeshComponent::getPoseController);
	registrar->registerClass(classAnimatedMeshComponent);

	auto classPoseController = new AutoRuntimeClass< IPoseController >();
	registrar->registerClass(classPoseController);

	auto classIKPoseController = new AutoRuntimeClass< IKPoseController >();
	classIKPoseController->addConstructor< IPoseController*, uint32_t >();
	classIKPoseController->addProperty("neutralPoseController", &IKPoseController::getNeutralPoseController);
	registrar->registerClass(classIKPoseController);

	auto classRagDollPoseController = new AutoRuntimeClass< RagDollPoseController >();
	classRagDollPoseController->addProperty("enable", &RagDollPoseController::setEnable, &RagDollPoseController::isEnable);
	classRagDollPoseController->addProperty("limbs", &RagDollPoseController::getLimbs);
	registrar->registerClass(classRagDollPoseController);

	auto classStatePoseController = new AutoRuntimeClass< StatePoseController >();
	classStatePoseController->addMethod("setState", &StatePoseController::setState);
	classStatePoseController->addMethod("setCondition", &StatePoseController::setCondition);
	classStatePoseController->addMethod("setTime", &StatePoseController::setTime);
	classStatePoseController->addMethod("getTime", &StatePoseController::getTime);
	classStatePoseController->addMethod("setTimeFactor", &StatePoseController::setTimeFactor);
	classStatePoseController->addMethod("getTimeFactor", &StatePoseController::getTimeFactor);
	registrar->registerClass(classStatePoseController);

	auto classBoidsComponent = new AutoRuntimeClass< BoidsComponent >();
	registrar->registerClass(classBoidsComponent);

	auto classClothComponent = new AutoRuntimeClass< ClothComponent >();
	classClothComponent->addMethod("reset", &ClothComponent::reset);
	classClothComponent->addMethod("setNodeInvMass", &ClothComponent::setNodeInvMass);
	registrar->registerClass(classClothComponent);

	auto classPathComponent = new AutoRuntimeClass< PathComponent >();
	classPathComponent->addMethod("continueTo", &PathComponent::continueTo);
	registrar->registerClass(classPathComponent);

	auto classPathComponentData = new AutoRuntimeClass< PathComponentData >();
	registrar->registerClass(classPathComponentData);
}

	}
}
