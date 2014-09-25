#include "Amalgam/Engine/Classes/AnimationClasses.h"
#include "Animation/AnimatedMeshEntity.h"
#include "Animation/Animation/StatePoseController.h"
#include "Animation/IK/IKPoseController.h"
#include "Animation/RagDoll/RagDollPoseController.h"
#include "Physics/Body.h"
#include "Physics/PhysicsManager.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

Transform animation_AnimatedMeshEntity_getJointTransform(animation::AnimatedMeshEntity* this_, const std::wstring& boneName)
{
	Transform transform;
	this_->getJointTransform(
		render::getParameterHandle(boneName),
		transform
	);
	return transform;
}

Transform animation_AnimatedMeshEntity_getPoseTransform(animation::AnimatedMeshEntity* this_, const std::wstring& boneName)
{
	Transform transform;
	this_->getPoseTransform(
		render::getParameterHandle(boneName),
		transform
	);
	return transform;
}

Transform animation_AnimatedMeshEntity_getSkinTransform(animation::AnimatedMeshEntity* this_, const std::wstring& boneName)
{
	Transform transform;
	this_->getSkinTransform(
		render::getParameterHandle(boneName),
		transform
	);
	return transform;
}

		}

void registerAnimationClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< animation::AnimatedMeshEntity > > classAnimatedMeshEntity = new script::AutoScriptClass< animation::AnimatedMeshEntity >();
	classAnimatedMeshEntity->addMethod("getJointTransform", &animation_AnimatedMeshEntity_getJointTransform);
	classAnimatedMeshEntity->addMethod("getPoseTransform", &animation_AnimatedMeshEntity_getPoseTransform);
	classAnimatedMeshEntity->addMethod("getSkinTransform", &animation_AnimatedMeshEntity_getSkinTransform);
	classAnimatedMeshEntity->addMethod("setPoseTransform", &animation::AnimatedMeshEntity::setPoseTransform);
	classAnimatedMeshEntity->addMethod("setPoseController", &animation::AnimatedMeshEntity::setPoseController);
	classAnimatedMeshEntity->addMethod("getPoseController", &animation::AnimatedMeshEntity::getPoseController);
	scriptManager->registerClass(classAnimatedMeshEntity);

	Ref< script::AutoScriptClass< animation::IPoseController > > classPoseController = new script::AutoScriptClass< animation::IPoseController >();
	classPoseController->addMethod("setTransform", &animation::IPoseController::setTransform);
	scriptManager->registerClass(classPoseController);

	Ref< script::AutoScriptClass< animation::IKPoseController > > classIKPoseController = new script::AutoScriptClass< animation::IKPoseController >();
	classIKPoseController->addConstructor< physics::PhysicsManager*, animation::IPoseController*, uint32_t >();
	classIKPoseController->addMethod("setIgnoreBody", &animation::IKPoseController::setIgnoreBody);
	classIKPoseController->addMethod("getNeutralPoseController", &animation::IKPoseController::getNeutralPoseController);
	scriptManager->registerClass(classIKPoseController);

	Ref< script::AutoScriptClass< animation::RagDollPoseController > > classRagDollPoseController = new script::AutoScriptClass< animation::RagDollPoseController >();
	classRagDollPoseController->addMethod("setEnable", &animation::RagDollPoseController::setEnable);
	classRagDollPoseController->addMethod("isEnable", &animation::RagDollPoseController::isEnable);
	classRagDollPoseController->addMethod("getLimbs", &animation::RagDollPoseController::getLimbs);
	scriptManager->registerClass(classRagDollPoseController);

	Ref< script::AutoScriptClass< animation::StatePoseController > > classStatePoseController = new script::AutoScriptClass< animation::StatePoseController >();
	classStatePoseController->addMethod("setState", &animation::StatePoseController::setState);
	classStatePoseController->addMethod("setCondition", &animation::StatePoseController::setCondition);
	classStatePoseController->addMethod("setTimeFactor", &animation::StatePoseController::setTimeFactor);
	scriptManager->registerClass(classStatePoseController);
}

	}
}
