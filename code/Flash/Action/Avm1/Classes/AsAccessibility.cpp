#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsAccessibility.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsAccessibility", AsAccessibility, ActionClass)

AsAccessibility::AsAccessibility()
:	ActionClass("Accessibility")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("isActive", ActionValue(createNativeFunction(this, &AsAccessibility::Accessibility_isActive)));
	prototype->setMember("updateProperties", ActionValue(createNativeFunction(this, &AsAccessibility::Accessibility_updateProperties)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

Ref< ActionObject > AsAccessibility::alloc(ActionContext* context)
{
	return new ActionObject("Accessibility");
}

void AsAccessibility::init(ActionContext* context, ActionObject* self, const ActionValueArray& args)
{
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
