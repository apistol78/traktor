#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsBoolean.h"
#include "Flash/Action/Classes/Boolean.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsBoolean", AsBoolean, ActionClass)

AsBoolean::AsBoolean(ActionContext* context)
:	ActionClass(context, "Boolean")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("toString", ActionValue(createNativeFunction(context, this, &AsBoolean::Boolean_toString)));
	prototype->setMember("valueOf", ActionValue(createNativeFunction(context, this, &AsBoolean::Boolean_valueOf)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsBoolean::initialize(ActionObject* self)
{
}

void AsBoolean::construct(ActionObject* self, const ActionValueArray& args)
{
	Ref< Boolean > b;
	if (args.size() > 0)
		b = new Boolean(args[0].getBoolean());
	else
		b = new Boolean(false);
	self->setRelay(b);
}

ActionValue AsBoolean::xplicit(const ActionValueArray& args)
{
	if (args.size() > 0)
		return ActionValue(args[0].getBoolean());
	else
		return ActionValue();
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
