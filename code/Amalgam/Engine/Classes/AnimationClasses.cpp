#include "Amalgam/Engine/Classes/AnimationClasses.h"
#include "Animation/AnimatedMeshEntity.h"
#include "Animation/Animation/StatePoseController.h"
#include "Animation/IK/IKPoseController.h"
#include "Animation/PathEntity/PathEntity.h"
#include "Animation/PathEntity/PathEntityData.h"
#include "Animation/RagDoll/RagDollPoseController.h"
#include "Physics/Body.h"
#include "Physics/PhysicsManager.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptDelegate.h"
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

void animation_PathEntity_setTimeMode(animation::PathEntity* self, const std::wstring& timeMode)
{
	if (timeMode == L"manual")
		self->setTimeMode(animation::PathEntity::TmManual);
	else if (timeMode == L"once")
		self->setTimeMode(animation::PathEntity::TmOnce);
	else if (timeMode == L"loop")
		self->setTimeMode(animation::PathEntity::TmLoop);
	else if (timeMode == L"pingPong")
		self->setTimeMode(animation::PathEntity::TmPingPong);
}

std::wstring animation_PathEntity_getTimeMode(animation::PathEntity* self)
{
	switch (self->getTimeMode())
	{
	case animation::PathEntity::TmManual:
		return L"manual";
	case animation::PathEntity::TmOnce:
		return L"once";
	case animation::PathEntity::TmLoop:
		return L"loop";
	case animation::PathEntity::TmPingPong:
		return L"pingPong";
	default:
		return L"";
	}
}

class DelegatePathEntityListener : public RefCountImpl< animation::PathEntity::IListener >
{
public:
	DelegatePathEntityListener(script::IScriptDelegate* delegate)
	:	m_delegate(delegate)
	{
	}

	virtual void notifyPathFinished(animation::PathEntity* entity)
	{
		script::Any argv[] =
		{
			script::CastAny< animation::PathEntity* >::set(entity)
		};
		m_delegate->call(sizeof_array(argv), argv);
	}

private:
	Ref< script::IScriptDelegate > m_delegate;
};

void animation_PathEntity_setListener(animation::PathEntity* self, script::IScriptDelegate* listener)
{
	self->setListener(new DelegatePathEntityListener(listener));
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
	classStatePoseController->addMethod("setTime", &animation::StatePoseController::setTime);
	classStatePoseController->addMethod("getTime", &animation::StatePoseController::getTime);
	classStatePoseController->addMethod("setTimeFactor", &animation::StatePoseController::setTimeFactor);
	classStatePoseController->addMethod("getTimeFactor", &animation::StatePoseController::getTimeFactor);
	scriptManager->registerClass(classStatePoseController);

	Ref< script::AutoScriptClass< animation::PathEntity > > classPathEntity = new script::AutoScriptClass< animation::PathEntity >();
	classPathEntity->addMethod("setTimeMode", &animation_PathEntity_setTimeMode);
	classPathEntity->addMethod("getTimeMode", &animation_PathEntity_getTimeMode);
	classPathEntity->addMethod("setTimeScale", &animation::PathEntity::setTimeScale);
	classPathEntity->addMethod("getTimeScale", &animation::PathEntity::getTimeScale);
	classPathEntity->addMethod("setTime", &animation::PathEntity::setTime);
	classPathEntity->addMethod("getTime", &animation::PathEntity::getTime);
	classPathEntity->addMethod("getEntity", &animation::PathEntity::getEntity);
	classPathEntity->addMethod("setListener", &animation_PathEntity_setListener);
	scriptManager->registerClass(classPathEntity);

	Ref< script::AutoScriptClass< animation::PathEntityData > > classPathEntityData = new script::AutoScriptClass< animation::PathEntityData >();
	classPathEntityData->addConstructor();
	scriptManager->registerClass(classPathEntityData);
}

	}
}
