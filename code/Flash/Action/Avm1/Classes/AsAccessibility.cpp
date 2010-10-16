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

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

ActionValue AsAccessibility::construct(ActionContext* context, const ActionValueArray& args)
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
