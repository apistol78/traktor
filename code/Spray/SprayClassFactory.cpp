#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Spray/EffectEntity.h"
#include "Spray/SprayClassFactory.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.SprayClassFactory", 0, SprayClassFactory, IRuntimeClassFactory)

void SprayClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< EffectEntity > > classEffectEntity = new AutoRuntimeClass< EffectEntity >();
	classEffectEntity->addMethod("isFinished", &EffectEntity::isFinished);
	classEffectEntity->addMethod("setEnable", &EffectEntity::setEnable);
	classEffectEntity->addMethod("isEnable", &EffectEntity::isEnable);
	registrar->registerClass(classEffectEntity);
}

	}
}
