#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsAccessibility.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsAccessibility", AsAccessibility, ActionClass)

AsAccessibility::AsAccessibility(ActionContext* context)
:	ActionClass(context, "Accessibility")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("isActive", ActionValue(createNativeFunction(context, this, &AsAccessibility::Accessibility_isActive)));
	prototype->setMember("updateProperties", ActionValue(createNativeFunction(context, this, &AsAccessibility::Accessibility_updateProperties)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsAccessibility::init(ActionObject* self, const ActionValueArray& args)
{
}

void AsAccessibility::coerce(ActionObject* self) const
{
	T_FATAL_ERROR;
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
