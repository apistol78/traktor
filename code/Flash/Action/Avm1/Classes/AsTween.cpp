#include "Flash/Action/Avm1/Classes/AsTween.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsTween", AsTween, ActionClass)

Ref< AsTween > AsTween::getInstance()
{
	static Ref< AsTween > instance = 0;
	if (!instance)
	{
		instance = new AsTween();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsTween::AsTween()
:	ActionClass(L"Tween")
{
}

void AsTween::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsTween::construct(ActionContext* context, const args_t& args)
{
	return ActionValue();
}

	}
}
