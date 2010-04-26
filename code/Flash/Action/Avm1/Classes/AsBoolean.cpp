#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsBoolean.h"
#include "Flash/Action/Avm1/Classes/AsObject.h"
#include "Flash/Action/Classes/Boolean.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsBoolean", AsBoolean, ActionClass)

Ref< AsBoolean > AsBoolean::getInstance()
{
	static Ref< AsBoolean > instance = 0;
	if (!instance)
	{
		instance = new AsBoolean();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsBoolean::AsBoolean()
:	ActionClass(L"Boolean")
{
}

void AsBoolean::createPrototype()
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember(L"__proto__", ActionValue(AsObject::getInstance()));
	prototype->setMember(L"toString", ActionValue(createNativeFunction(this, &AsBoolean::Boolean_toString)));
	prototype->setMember(L"valueOf", ActionValue(createNativeFunction(this, &AsBoolean::Boolean_valueOf)));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue(prototype));
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
