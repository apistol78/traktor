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

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

Ref< ActionObject > AsBoolean::alloc(ActionContext* context)
{
	return new Boolean(false);
}

void AsBoolean::init(ActionContext* context, ActionObject* self, const ActionValueArray& args)
{
	if (args.size() > 0)
		checked_type_cast< Boolean* >(self)->set(args[0].getBoolean());
}

std::string AsBoolean::Boolean_toString(Boolean* self) const
{
	return self->get() ? "true" : "false";
}

bool AsBoolean::Boolean_valueOf(Boolean* self) const
{
	return self->get();
}

	}
}
