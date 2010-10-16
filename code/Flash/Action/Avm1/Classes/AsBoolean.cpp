#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsBoolean.h"
#include "Flash/Action/Classes/Boolean.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsBoolean", AsBoolean, ActionClass)

AsBoolean::AsBoolean()
:	ActionClass("Boolean")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("toString", ActionValue(createNativeFunction(this, &AsBoolean::Boolean_toString)));
	prototype->setMember("valueOf", ActionValue(createNativeFunction(this, &AsBoolean::Boolean_valueOf)));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

ActionValue AsBoolean::construct(ActionContext* context, const ActionValueArray& args)
{
	if (args.empty())
		return ActionValue(new Boolean(false));
	else
		return ActionValue(new Boolean(args[0].getBooleanSafe()));
}

std::wstring AsBoolean::Boolean_toString(Boolean* self) const
{
	return self->get() ? L"true" : L"false";
}

bool AsBoolean::Boolean_valueOf(Boolean* self) const
{
	return self->get();
}

	}
}
