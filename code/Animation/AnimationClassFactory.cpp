/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/AnimationClassFactory.h"
#include "Animation/AnimatedMeshComponent.h"
#include "Animation/JointBindingComponent.h"
#include "Animation/SkeletonComponent.h"
#include "Animation/Animation/AnimationGraphPoseController.h"
#include "Animation/Animation/RetargetPoseController.h"
#include "Animation/Boids/BoidsComponent.h"
#include "Animation/Cloth/ClothComponent.h"
#include "Animation/IK/IKComponent.h"
#include "Animation/PathEntity/PathComponent.h"
#include "Animation/PathEntity/PathComponentData.h"
#include "Animation/RagDoll/RagDollPoseController.h"
#include "Animation/Rotator/RotatorComponent.h"
#include "Animation/Rotator/WobbleComponent.h"
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Class/Boxes/BoxedTransform.h"
#include "Core/Class/Boxes/BoxedVector4.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Physics/Body.h"
#include "Physics/PhysicsManager.h"
#include "World/Entity.h"

namespace traktor::animation
{
	namespace
	{

Transform SkeletonComponent_getJointTransform(SkeletonComponent* self, const std::wstring& jointName)
{
	Transform transform;
	self->getJointTransform(
		render::getParameterHandle(jointName),
		transform
	);
	return transform;
}

Transform SkeletonComponent_getPoseTransform(SkeletonComponent* self, const std::wstring& jointName)
{
	Transform transform;
	self->getPoseTransform(
		render::getParameterHandle(jointName),
		transform
	);
	return transform;
}

bool SkeletonComponent_setPoseTransform(SkeletonComponent* self, const std::wstring& jointName, const Transform& transform, bool inclusive)
{
	return self->setPoseTransform(
		render::getParameterHandle(jointName),
		transform,
		inclusive
	);
}

bool SkeletonComponent_concatenatePoseTransform(SkeletonComponent* self, const std::wstring& jointName, const Transform& transform, bool inclusive)
{
	return self->concatenatePoseTransform(
		render::getParameterHandle(jointName),
		transform,
		inclusive
	);
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

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationClassFactory", 0, AnimationClassFactory, IRuntimeClassFactory)

void AnimationClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classSkeletonComponent = new AutoRuntimeClass< SkeletonComponent >();
	classSkeletonComponent->addProperty("poseController", &SkeletonComponent::setPoseController, &SkeletonComponent::getPoseController);
	classSkeletonComponent->addMethod("getJointTransform", &SkeletonComponent_getJointTransform);
	classSkeletonComponent->addMethod("getPoseTransform", &SkeletonComponent_getPoseTransform);
	classSkeletonComponent->addMethod("setPoseTransform", &SkeletonComponent_setPoseTransform);
	classSkeletonComponent->addMethod("concatenatePoseTransform", &SkeletonComponent_concatenatePoseTransform);
	registrar->registerClass(classSkeletonComponent);

	auto classAnimatedMeshComponent = new AutoRuntimeClass< AnimatedMeshComponent >();
	classAnimatedMeshComponent->addMethod("getSkinTransform", &AnimatedMeshComponent_getSkinTransform);
	registrar->registerClass(classAnimatedMeshComponent);

	auto classIKComponent = new AutoRuntimeClass< IKComponent >();
	classIKComponent->addConstructor();
	registrar->registerClass(classIKComponent);

	auto classJointBindingComponent = new AutoRuntimeClass< JointBindingComponent >();
	classJointBindingComponent->addMethod("getEntity", &JointBindingComponent::getEntity);
	classJointBindingComponent->addMethod("getEntities", &JointBindingComponent::getEntities);
	registrar->registerClass(classJointBindingComponent);

	auto classPoseController = new AutoRuntimeClass< IPoseController >();
	registrar->registerClass(classPoseController);

	auto classRagDollPoseController = new AutoRuntimeClass< RagDollPoseController >();
	classRagDollPoseController->addProperty("enable", &RagDollPoseController::setEnable, &RagDollPoseController::isEnable);
	classRagDollPoseController->addProperty("limbs", &RagDollPoseController::getLimbs);
	registrar->registerClass(classRagDollPoseController);

	auto classStatePoseController = new AutoRuntimeClass< AnimationGraphPoseController >();
	classStatePoseController->addProperty("time", &AnimationGraphPoseController::setTime, &AnimationGraphPoseController::getTime);
	classStatePoseController->addProperty("timeFactor", &AnimationGraphPoseController::setTimeFactor, &AnimationGraphPoseController::getTimeFactor);
	classStatePoseController->addMethod("setState", &AnimationGraphPoseController::setState);
	classStatePoseController->addMethod("setCondition", &AnimationGraphPoseController::setCondition);
	registrar->registerClass(classStatePoseController);

	auto classRetargetPoseController = new AutoRuntimeClass< RetargetPoseController >();
	classRetargetPoseController->addProperty("controller", &RetargetPoseController::getController);
	registrar->registerClass(classRetargetPoseController);

	auto classBoidsComponent = new AutoRuntimeClass< BoidsComponent >();
	classBoidsComponent->addProperty("attractPosition", &BoidsComponent::setAttractPosition, &BoidsComponent::getAttractPosition);
	registrar->registerClass(classBoidsComponent);

	auto classClothComponent = new AutoRuntimeClass< ClothComponent >();
	classClothComponent->addMethod("reset", &ClothComponent::reset);
	registrar->registerClass(classClothComponent);

	auto classPathComponent = new AutoRuntimeClass< PathComponent >();
	classPathComponent->addMethod("continueTo", &PathComponent::continueTo);
	registrar->registerClass(classPathComponent);

	auto classPathComponentData = new AutoRuntimeClass< PathComponentData >();
	registrar->registerClass(classPathComponentData);

	auto classRotatorComponent = new AutoRuntimeClass< RotatorComponent >();
	registrar->registerClass(classRotatorComponent);

	auto classWobbleComponent = new AutoRuntimeClass< WobbleComponent >();
	registrar->registerClass(classWobbleComponent);
}

}
