#include "Animation/AnimatedMeshEntity.h"
#include "Animation/Animation/StatePoseController.h"
#include "Animation/IK/IKPoseController.h"
#include "Animation/RagDoll/RagDollPoseController.h"
#include "Parade/Classes/AnimationClasses.h"
#include "Physics/Body.h"
#include "Physics/PhysicsManager.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

Transform animation_AnimatedMeshEntity_getBoneTransform(animation::AnimatedMeshEntity* this_, const std::wstring& boneName)
{
	Transform transform;
	this_->getBoneTransform(boneName, transform);
	return transform;
}

Transform animation_AnimatedMeshEntity_getPoseTransform(animation::AnimatedMeshEntity* this_, const std::wstring& boneName)
{
	Transform transform;
	this_->getPoseTransform(boneName, transform);
	return transform;
}

Transform animation_AnimatedMeshEntity_getSkinTransform(animation::AnimatedMeshEntity* this_, const std::wstring& boneName)
{
	Transform transform;
	this_->getSkinTransform(boneName, transform);
	return transform;
}

		}

void registerAnimationClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< animation::AnimatedMeshEntity > > classAnimatedMeshEntity = new script::AutoScriptClass< animation::AnimatedMeshEntity >();
	classAnimatedMeshEntity->addMethod(L"getBoneTransform", &animation_AnimatedMeshEntity_getBoneTransform);
	classAnimatedMeshEntity->addMethod(L"getPoseTransform", &animation_AnimatedMeshEntity_getPoseTransform);
	classAnimatedMeshEntity->addMethod(L"getSkinTransform", &animation_AnimatedMeshEntity_getSkinTransform);
	classAnimatedMeshEntity->addMethod(L"setPoseTransform", &animation::AnimatedMeshEntity::setPoseTransform);
	classAnimatedMeshEntity->addMethod(L"setPoseController", &animation::AnimatedMeshEntity::setPoseController);
	classAnimatedMeshEntity->addMethod(L"getPoseController", &animation::AnimatedMeshEntity::getPoseController);
	scriptManager->registerClass(classAnimatedMeshEntity);

	Ref< script::AutoScriptClass< animation::IPoseController > > classPoseController = new script::AutoScriptClass< animation::IPoseController >();
	classPoseController->addMethod(L"setTransform", &animation::IPoseController::setTransform);
	scriptManager->registerClass(classPoseController);

	Ref< script::AutoScriptClass< animation::IKPoseController > > classIKPoseController = new script::AutoScriptClass< animation::IKPoseController >();
	classIKPoseController->addConstructor< physics::PhysicsManager*, animation::IPoseController*, uint32_t >();
	classIKPoseController->addMethod(L"setIgnoreBody", &animation::IKPoseController::setIgnoreBody);
	classIKPoseController->addMethod(L"getNeutralPoseController", &animation::IKPoseController::getNeutralPoseController);
	scriptManager->registerClass(classIKPoseController);

	Ref< script::AutoScriptClass< animation::RagDollPoseController > > classRagDollPoseController = new script::AutoScriptClass< animation::RagDollPoseController >();
	classRagDollPoseController->addMethod(L"setEnable", &animation::RagDollPoseController::setEnable);
	classRagDollPoseController->addMethod(L"isEnable", &animation::RagDollPoseController::isEnable);
	classRagDollPoseController->addMethod(L"getLimbs", &animation::RagDollPoseController::getLimbs);
	scriptManager->registerClass(classRagDollPoseController);

	Ref< script::AutoScriptClass< animation::StatePoseController > > classStatePoseController = new script::AutoScriptClass< animation::StatePoseController >();
	classStatePoseController->addMethod(L"setCondition", &animation::StatePoseController::setCondition);
	classStatePoseController->addMethod(L"setTimeFactor", &animation::StatePoseController::setTimeFactor);
	scriptManager->registerClass(classStatePoseController);
}

	}
}
