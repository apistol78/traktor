#include "Amalgam/Engine/Classes/SprayClasses.h"
#include "Spray/EffectEntity.h"
#include "Script/AutoScriptClass.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace amalgam
	{

void registerSprayClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< spray::EffectEntity > > classEffectEntity = new script::AutoScriptClass< spray::EffectEntity >();
	classEffectEntity->addMethod("isFinished", &spray::EffectEntity::isFinished);
	classEffectEntity->addMethod("setEnable", &spray::EffectEntity::setEnable);
	classEffectEntity->addMethod("isEnable", &spray::EffectEntity::isEnable);
	scriptManager->registerClass(classEffectEntity);
}

	}
}
