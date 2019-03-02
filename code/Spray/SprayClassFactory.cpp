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
	auto classEffectComponent = new AutoRuntimeClass< EffectComponent >();
	classEffectComponent->addProperty("loopEnable", &EffectComponent::setLoopEnable, &EffectComponent::getLoopEnable);
	classEffectComponent->addProperty("finished", &EffectComponent::isFinished);
	classEffectComponent->addProperty("enable", &EffectComponent::setEnable, &EffectComponent::isEnable);
	registrar->registerClass(classEffectComponent);

	auto classEffectEntity = new AutoRuntimeClass< EffectEntity >();
	classEffectEntity->addProperty("loopEnable", &EffectEntity::setLoopEnable, &EffectEntity::getLoopEnable);
	classEffectEntity->addProperty("finished", &EffectEntity::isFinished);
	classEffectEntity->addProperty("enable", &EffectEntity::setEnable, &EffectEntity::isEnable);
	registrar->registerClass(classEffectEntity);
}

	}
}
