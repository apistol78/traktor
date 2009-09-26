#include <limits>
#include "Flash/Action/Classes/AsNumber.h"
#include "Flash/Action/Classes/AsObject.h"
#include "Flash/Action/ActionNumber.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Core/Misc/StringUtilities.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsNumber", AsNumber, ActionClass)

AsNumber* AsNumber::getInstance()
{
	static AsNumber* instance = 0;
	if (!instance)
	{
		instance = new AsNumber();
		instance->createPrototype();
		instance->setReadOnly();
	}
	return instance;
}

AsNumber::AsNumber()
:	ActionClass(L"Number")
{
}

void AsNumber::createPrototype()
{
	Ref< ActionObject > prototype = gc_new< ActionObject >();

	prototype->setMember(L"__proto__", ActionValue::fromObject(AsObject::getInstance()));
	prototype->setMember(L"MAX_VALUE", ActionValue(std::numeric_limits< double >::max()));
	prototype->setMember(L"MIN_VALUE", ActionValue(std::numeric_limits< double >::min()));
	prototype->setMember(L"NaN", ActionValue(std::numeric_limits< double >::signaling_NaN()));
	prototype->setMember(L"NEGATIVE_INFINITY", ActionValue(-std::numeric_limits< double >::infinity()));
	prototype->setMember(L"POSITIVE_INFINITY", ActionValue(std::numeric_limits< double >::infinity()));
	prototype->setMember(L"toString", createNativeFunctionValue(this, &AsNumber::Number_toString));
	prototype->setMember(L"valueOf", createNativeFunctionValue(this, &AsNumber::Number_valueOf));

	prototype->setReadOnly();

	setMember(L"prototype", ActionValue::fromObject(prototype));
}

ActionValue AsNumber::construct(ActionContext* context, const args_t& args)
{
	if (args.empty())
		return ActionValue::fromObject(gc_new< ActionNumber >(0.0));
	else
		return ActionValue::fromObject(gc_new< ActionNumber >(args[0].getNumberSafe()));
}

void AsNumber::Number_toString(CallArgs& ca)
{
	const ActionNumber* obj = checked_type_cast< const ActionNumber* >(ca.self);
	ca.ret = ActionValue(traktor::toString(obj->get()));
}

void AsNumber::Number_valueOf(CallArgs& ca)
{
	const ActionNumber* obj = checked_type_cast< const ActionNumber* >(ca.self);
	ca.ret = ActionValue(obj->get());
}

	}
}
