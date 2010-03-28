#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsAccessibility.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsAccessibility", AsAccessibility, ActionClass)

Ref< AsAccessibility > AsAccessibility::getInstance()
{
	static Ref< AsAccessibility > instance = 0;
	if (!instance)
	{
		instance = new AsAccessibility();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsAccessibility::AsAccessibility()
:	ActionClass(L"Accessibility")
{
}

void AsAccessibility::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"isActive", ActionValue(createNativeFunction(this, &AsAccessibility::Accessibility_isActive)));
	prototype->setMember(L"updateProperties", ActionValue(createNativeFunction(this, &AsAccessibility::Accessibility_updateProperties)));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
}

ActionValue AsAccessibility::construct(ActionContext* context, const args_t& args)
{
	return ActionValue();
}

void AsAccessibility::Accessibility_isActive(CallArgs& ca)
{
	ca.ret = ActionValue(false);
}

void AsAccessibility::Accessibility_updateProperties(CallArgs& ca)
{
}

	}
}
