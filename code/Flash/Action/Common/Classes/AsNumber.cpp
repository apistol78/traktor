#include <limits>
#include "Core/Misc/String.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/Number.h"
#include "Flash/Action/Common/Classes/AsNumber.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsNumber", AsNumber, ActionClass)

AsNumber::AsNumber(ActionContext* context)
:	ActionClass(context, "Number")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("MAX_VALUE", ActionValue(std::numeric_limits< float >::max()));
	prototype->setMember("MIN_VALUE", ActionValue(std::numeric_limits< float >::min()));
	prototype->setMember("NaN", ActionValue(std::numeric_limits< float >::signaling_NaN()));
	prototype->setMember("NEGATIVE_INFINITY", ActionValue(-std::numeric_limits< float >::infinity()));
	prototype->setMember("POSITIVE_INFINITY", ActionValue(std::numeric_limits< float >::infinity()));
	prototype->setMember("toString", ActionValue(createNativeFunction(context, this, &AsNumber::Number_toString)));
	prototype->setMember("valueOf", ActionValue(createNativeFunction(context, this, &AsNumber::Number_valueOf)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsNumber::initialize(ActionObject* self)
{
}

void AsNumber::construct(ActionObject* self, const ActionValueArray& args)
{
	Ref< Number > n;
	if (args.size() > 0)
		n = new Number(args[0].getFloat());
	else
		n = new Number(0.0f);
	self->setRelay(n);
}

ActionValue AsNumber::xplicit(const ActionValueArray& args)
{
	if (args.size() > 0)
		return ActionValue(args[0].getFloat());
	else
		return ActionValue();
}

std::wstring AsNumber::Number_toString(const Number* self) const
{
	return traktor::toString(self->get());
}

float AsNumber::Number_valueOf(const Number* self) const
{
	return self->get();
}

	}
}
