#include "Animation/AnimationClassFactory.h"
#include "Animation/AnimatedMeshEntity.h"
#include "Animation/Animation/StatePoseController.h"
#include "Animation/Boids/BoidsEntity.h"
#include "Animation/Cloth/ClothEntity.h"
#include "Animation/IK/IKPoseController.h"
#include "Animation/PathEntity/PathEntity.h"
#include "Animation/PathEntity/PathEntityData.h"
#include "Animation/PathEntity/PathComponent.h"
#include "Animation/PathEntity/PathComponentData.h"
#include "Animation/RagDoll/RagDollPoseController.h"
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
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

Transform animation_AnimatedMeshEntity_getJointTransform(AnimatedMeshEntity* this_, const std::wstring& boneName)
{
	Transform transform;
	this_->getJointTransform(
		render::getParameterHandle(boneName),
		transform
	);
	return transform;
}

Transform animation_AnimatedMeshEntity_getPoseTransform(AnimatedMeshEntity* this_, const std::wstring& boneName)
{
	Transform transform;
	this_->getPoseTransform(
		render::getParameterHandle(boneName),
		transform
	);
	return transform;
}

Transform animation_AnimatedMeshEntity_getSkinTransform(AnimatedMeshEntity* this_, const std::wstring& boneName)
{
	Transform transform;
	this_->getSkinTransform(
		render::getParameterHandle(boneName),
		transform
	);
	return transform;
}

void animation_PathEntity_setTimeMode(PathEntity* self, const std::wstring& timeMode)
{
	if (timeMode == L"manual")
		self->setTimeMode(PathEntity::TmManual);
	else if (timeMode == L"once")
		self->setTimeMode(PathEntity::TmOnce);
	else if (timeMode == L"loop")
		self->setTimeMode(PathEntity::TmLoop);
	else if (timeMode == L"pingPong")
		self->setTimeMode(PathEntity::TmPingPong);
}

std::wstring animation_PathEntity_getTimeMode(PathEntity* self)
{
	switch (self->getTimeMode())
	{
	case PathEntity::TmManual:
		return L"manual";
	case PathEntity::TmOnce:
		return L"once";
	case PathEntity::TmLoop:
		return L"loop";
	case PathEntity::TmPingPong:
		return L"pingPong";
	default:
		return L"";
	}
}

class DelegatePathEntityListener : public RefCountImpl< PathEntity::IListener >
{
public:
	DelegatePathEntityListener(IRuntimeDelegate* delegate)
	:	m_delegate(delegate)
	{
	}

	virtual void notifyPathFinished(PathEntity* entity)
	{
		Any argv[] =
		{
			CastAny< PathEntity* >::set(entity)
		};
		m_delegate->call(sizeof_array(argv), argv);
	}

private:
	Ref< IRuntimeDelegate > m_delegate;
};

void animation_PathEntity_setListener(PathEntity* self, IRuntimeDelegate* listener)
{
	self->setListener(new DelegatePathEntityListener(listener));
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.AnimationClassFactory", 0, AnimationClassFactory, IRuntimeClassFactory)

void AnimationClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< AnimatedMeshEntity > > classAnimatedMeshEntity = new AutoRuntimeClass< AnimatedMeshEntity >();
	classAnimatedMeshEntity->addMethod("getJointTransform", &animation_AnimatedMeshEntity_getJointTransform);
	classAnimatedMeshEntity->addMethod("getPoseTransform", &animation_AnimatedMeshEntity_getPoseTransform);
	classAnimatedMeshEntity->addMethod("getSkinTransform", &animation_AnimatedMeshEntity_getSkinTransform);
	classAnimatedMeshEntity->addMethod("setPoseTransform", &AnimatedMeshEntity::setPoseTransform);
	classAnimatedMeshEntity->addMethod("setPoseController", &AnimatedMeshEntity::setPoseController);
	classAnimatedMeshEntity->addMethod("getPoseController", &AnimatedMeshEntity::getPoseController);
	registrar->registerClass(classAnimatedMeshEntity);

	Ref< AutoRuntimeClass< IPoseController > > classPoseController = new AutoRuntimeClass< IPoseController >();
	classPoseController->addMethod("setTransform", &IPoseController::setTransform);
	registrar->registerClass(classPoseController);

	Ref< AutoRuntimeClass< IKPoseController > > classIKPoseController = new AutoRuntimeClass< IKPoseController >();
	classIKPoseController->addConstructor< physics::PhysicsManager*, IPoseController*, uint32_t >();
	classIKPoseController->addMethod("setIgnoreBody", &IKPoseController::setIgnoreBody);
	classIKPoseController->addMethod("getNeutralPoseController", &IKPoseController::getNeutralPoseController);
	registrar->registerClass(classIKPoseController);

	Ref< AutoRuntimeClass< RagDollPoseController > > classRagDollPoseController = new AutoRuntimeClass< RagDollPoseController >();
	classRagDollPoseController->addMethod("setEnable", &RagDollPoseController::setEnable);
	classRagDollPoseController->addMethod("isEnable", &RagDollPoseController::isEnable);
	classRagDollPoseController->addMethod("getLimbs", &RagDollPoseController::getLimbs);
	registrar->registerClass(classRagDollPoseController);

	Ref< AutoRuntimeClass< StatePoseController > > classStatePoseController = new AutoRuntimeClass< StatePoseController >();
	classStatePoseController->addMethod("setState", &StatePoseController::setState);
	classStatePoseController->addMethod("setCondition", &StatePoseController::setCondition);
	classStatePoseController->addMethod("setTime", &StatePoseController::setTime);
	classStatePoseController->addMethod("getTime", &StatePoseController::getTime);
	classStatePoseController->addMethod("setTimeFactor", &StatePoseController::setTimeFactor);
	classStatePoseController->addMethod("getTimeFactor", &StatePoseController::getTimeFactor);
	registrar->registerClass(classStatePoseController);

	Ref< AutoRuntimeClass< BoidsEntity > > classBoidsEntity = new AutoRuntimeClass< BoidsEntity >();
	registrar->registerClass(classBoidsEntity);

	Ref< AutoRuntimeClass< ClothEntity > > classClothEntity = new AutoRuntimeClass< ClothEntity >();
	classClothEntity->addMethod("reset", &ClothEntity::reset);
	classClothEntity->addMethod("setNodeInvMass", &ClothEntity::setNodeInvMass);
	registrar->registerClass(classClothEntity);

	Ref< AutoRuntimeClass< PathEntity > > classPathEntity = new AutoRuntimeClass< PathEntity >();
	classPathEntity->addMethod("setTimeMode", &animation_PathEntity_setTimeMode);
	classPathEntity->addMethod("getTimeMode", &animation_PathEntity_getTimeMode);
	classPathEntity->addMethod("setTimeScale", &PathEntity::setTimeScale);
	classPathEntity->addMethod("getTimeScale", &PathEntity::getTimeScale);
	classPathEntity->addMethod("setTime", &PathEntity::setTime);
	classPathEntity->addMethod("getTime", &PathEntity::getTime);
	classPathEntity->addMethod("getEntity", &PathEntity::getEntity);
	classPathEntity->addMethod("setListener", &animation_PathEntity_setListener);
	registrar->registerClass(classPathEntity);

	Ref< AutoRuntimeClass< PathEntityData > > classPathEntityData = new AutoRuntimeClass< PathEntityData >();
	classPathEntityData->addConstructor();
	registrar->registerClass(classPathEntityData);

	Ref< AutoRuntimeClass< PathComponent > > classPathComponent = new AutoRuntimeClass< PathComponent >();
	registrar->registerClass(classPathComponent);

	Ref< AutoRuntimeClass< PathComponentData > > classPathComponentData = new AutoRuntimeClass< PathComponentData >();
	registrar->registerClass(classPathComponentData);
}

	}
}
