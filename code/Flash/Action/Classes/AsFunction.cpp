#include "Flash/Action/Classes/AsFunction.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Flash/Action/ActionFunctionNative.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsFunction", AsFunction, ActionClass)

Ref< AsFunction > AsFunction::getInstance()
{
	static AsFunction* instance = 0;
	if (!instance)
	{
		instance = new AsFunction();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsFunction::AsFunction()
:	ActionClass(L"Function")
{
}

void AsFunction::createPrototype()
{
	Ref< ActionObject > prototype = gc_new< ActionObject >();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsFunction::construct(ActionContext* context, const args_t& args)
{
	return ActionValue();
}

	}
}
