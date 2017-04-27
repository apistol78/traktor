/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Spray/EffectComponent.h"
#include "Spray/EffectEntity.h"
#include "Spray/SprayClassFactory.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.SprayClassFactory", 0, SprayClassFactory, IRuntimeClassFactory)

void SprayClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< EffectComponent > > classEffectComponent = new AutoRuntimeClass< EffectComponent >();
	classEffectComponent->addMethod("setLoopEnable", &EffectComponent::setLoopEnable);
	classEffectComponent->addMethod("getLoopEnable", &EffectComponent::getLoopEnable);
	classEffectComponent->addMethod("isFinished", &EffectComponent::isFinished);
	classEffectComponent->addMethod("setEnable", &EffectComponent::setEnable);
	classEffectComponent->addMethod("isEnable", &EffectComponent::isEnable);
	registrar->registerClass(classEffectComponent);

	Ref< AutoRuntimeClass< EffectEntity > > classEffectEntity = new AutoRuntimeClass< EffectEntity >();
	classEffectEntity->addMethod("setLoopEnable", &EffectEntity::setLoopEnable);
	classEffectEntity->addMethod("getLoopEnable", &EffectEntity::getLoopEnable);
	classEffectEntity->addMethod("isFinished", &EffectEntity::isFinished);
	classEffectEntity->addMethod("setEnable", &EffectEntity::setEnable);
	classEffectEntity->addMethod("isEnable", &EffectEntity::isEnable);
	registrar->registerClass(classEffectEntity);
}

	}
}
