#include <limits>
#include "Core/Misc/String.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/Number.h"
#include "Flash/Action/Avm1/Classes/AsNumber.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsNumber", AsNumber, ActionClass)

AsNumber::AsNumber()
:	ActionClass("Number")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("MAX_VALUE", ActionValue(std::numeric_limits< avm_number_t >::max()));
	prototype->setMember("MIN_VALUE", ActionValue(std::numeric_limits< avm_number_t >::min()));
	prototype->setMember("NaN", ActionValue(std::numeric_limits< avm_number_t >::signaling_NaN()));
	prototype->setMember("NEGATIVE_INFINITY", ActionValue(-std::numeric_limits< avm_number_t >::infinity()));
	prototype->setMember("POSITIVE_INFINITY", ActionValue(std::numeric_limits< avm_number_t >::infinity()));
	prototype->setMember("toString", ActionValue(createNativeFunction(this, &AsNumber::Number_toString)));
	prototype->setMember("valueOf", ActionValue(createNativeFunction(this, &AsNumber::Number_valueOf)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

Ref< ActionObject > AsNumber::alloc(ActionContext* context)
{
	return new Number(avm_number_t(0));
}

void AsNumber::init(ActionContext* context, ActionObject* self, const ActionValueArray& args)
{
	if (args.size() > 0)
		checked_type_cast< Number* >(self)->set(args[0].getNumber());
}

std::wstring AsNumber::Number_toString(const Number* self) const
{
	return traktor::toString(self->get());
}

avm_number_t AsNumber::Number_valueOf(const Number* self) const
{
	return self->get();
}

	}
}
