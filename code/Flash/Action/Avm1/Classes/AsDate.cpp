#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/Date.h"
#include "Flash/Action/Avm1/Classes/AsDate.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsDate", AsDate, ActionClass)

AsDate::AsDate(ActionContext* context)
:	ActionClass(context, "Date")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("getDate", ActionValue(createNativeFunction(context, this, &AsDate::Date_getDate)));
	prototype->setMember("getDay", ActionValue(createNativeFunction(context, this, &AsDate::Date_getDay)));
	prototype->setMember("getFullYear", ActionValue(createNativeFunction(context, this, &AsDate::Date_getFullYear)));
	prototype->setMember("getHours", ActionValue(createNativeFunction(context, this, &AsDate::Date_getHours)));
	prototype->setMember("getMilliseconds", ActionValue(createNativeFunction(context, this, &AsDate::Date_getMilliseconds)));
	prototype->setMember("getMinutes", ActionValue(createNativeFunction(context, this, &AsDate::Date_getMinutes)));
	prototype->setMember("getMonth", ActionValue(createNativeFunction(context, this, &AsDate::Date_getMonth)));
	prototype->setMember("getSeconds", ActionValue(createNativeFunction(context, this, &AsDate::Date_getSeconds)));
	prototype->setMember("getTime", ActionValue(createNativeFunction(context, this, &AsDate::Date_getTime)));
	prototype->setMember("getTimezoneOffset", ActionValue(createNativeFunction(context, this, &AsDate::Date_getTimezoneOffset)));
	prototype->setMember("getUTCDate", ActionValue(createNativeFunction(context, this, &AsDate::Date_getUTCDate)));
	prototype->setMember("getUTCDay", ActionValue(createNativeFunction(context, this, &AsDate::Date_getUTCDay)));
	prototype->setMember("getUTCFullYear", ActionValue(createNativeFunction(context, this, &AsDate::Date_getUTCFullYear)));
	prototype->setMember("getUTCHours", ActionValue(createNativeFunction(context, this, &AsDate::Date_getUTCHours)));
	prototype->setMember("getUTCMilliseconds", ActionValue(createNativeFunction(context, this, &AsDate::Date_getUTCMilliseconds)));
	prototype->setMember("getUTCMinutes", ActionValue(createNativeFunction(context, this, &AsDate::Date_getUTCMinutes)));
	prototype->setMember("getUTCMonth", ActionValue(createNativeFunction(context, this, &AsDate::Date_getUTCMonth)));
	prototype->setMember("getUTCSeconds", ActionValue(createNativeFunction(context, this, &AsDate::Date_getUTCSeconds)));
	prototype->setMember("getUTCYear", ActionValue(createNativeFunction(context, this, &AsDate::Date_getUTCYear)));
	prototype->setMember("getYear", ActionValue(createNativeFunction(context, this, &AsDate::Date_getYear)));
	prototype->setMember("setDate", ActionValue(createNativeFunction(context, this, &AsDate::Date_setDate)));
	prototype->setMember("setFullYear", ActionValue(createNativeFunction(context, this, &AsDate::Date_setFullYear)));
	prototype->setMember("setHours", ActionValue(createNativeFunction(context, this, &AsDate::Date_setHours)));
	prototype->setMember("setMilliseconds", ActionValue(createNativeFunction(context, this, &AsDate::Date_setMilliseconds)));
	prototype->setMember("setMinutes", ActionValue(createNativeFunction(context, this, &AsDate::Date_setMinutes)));
	prototype->setMember("setMonth", ActionValue(createNativeFunction(context, this, &AsDate::Date_setMonth)));
	prototype->setMember("setSeconds", ActionValue(createNativeFunction(context, this, &AsDate::Date_setSeconds)));
	prototype->setMember("setTime", ActionValue(createNativeFunction(context, this, &AsDate::Date_setTime)));
	prototype->setMember("setUTCDate", ActionValue(createNativeFunction(context, this, &AsDate::Date_setUTCDate)));
	prototype->setMember("setUTCFullYear", ActionValue(createNativeFunction(context, this, &AsDate::Date_setUTCFullYear)));
	prototype->setMember("setUTCHours", ActionValue(createNativeFunction(context, this, &AsDate::Date_setUTCHours)));
	prototype->setMember("setUTCMilliseconds", ActionValue(createNativeFunction(context, this, &AsDate::Date_setUTCMilliseconds)));
	prototype->setMember("setUTCMinutes", ActionValue(createNativeFunction(context, this, &AsDate::Date_setUTCMinutes)));
	prototype->setMember("setUTCMonth", ActionValue(createNativeFunction(context, this, &AsDate::Date_setUTCMonth)));
	prototype->setMember("setUTCSeconds", ActionValue(createNativeFunction(context, this, &AsDate::Date_setUTCSeconds)));
	prototype->setMember("setYear", ActionValue(createNativeFunction(context, this, &AsDate::Date_setYear)));
	prototype->setMember("toString", ActionValue(createNativeFunction(context, this, &AsDate::Date_toString)));
	prototype->setMember("UTC", ActionValue(createNativeFunction(context, this, &AsDate::Date_UTC)));
	prototype->setMember("valueOf", ActionValue(createNativeFunction(context, this, &AsDate::Date_valueOf)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsDate::init(ActionObject* self, const ActionValueArray& args) const
{
	self->setRelay(new Date());
}

void AsDate::coerce(ActionObject* self) const
{
	T_FATAL_ERROR;
}

void AsDate::Date_getDate(const Date* self) const
{
}

void AsDate::Date_getDay(const Date* self) const
{
}

void AsDate::Date_getFullYear(const Date* self) const
{
}

void AsDate::Date_getHours(const Date* self) const
{
}

void AsDate::Date_getMilliseconds(const Date* self) const
{
}

void AsDate::Date_getMinutes(const Date* self) const
{
}

void AsDate::Date_getMonth(const Date* self) const
{
}

void AsDate::Date_getSeconds(const Date* self) const
{
}

void AsDate::Date_getTime(const Date* self) const
{
}

void AsDate::Date_getTimezoneOffset(const Date* self) const
{
}

void AsDate::Date_getUTCDate(const Date* self) const
{
}

void AsDate::Date_getUTCDay(const Date* self) const
{
}

void AsDate::Date_getUTCFullYear(const Date* self) const
{
}

void AsDate::Date_getUTCHours(const Date* self) const
{
}

void AsDate::Date_getUTCMilliseconds(const Date* self) const
{
}

void AsDate::Date_getUTCMinutes(const Date* self) const
{
}

void AsDate::Date_getUTCMonth(const Date* self) const
{
}

void AsDate::Date_getUTCSeconds(const Date* self) const
{
}

void AsDate::Date_getUTCYear(const Date* self) const
{
}

void AsDate::Date_getYear(const Date* self) const
{
}

void AsDate::Date_setDate(Date* self) const
{
}

void AsDate::Date_setFullYear(Date* self) const
{
}

void AsDate::Date_setHours(Date* self) const
{
}

void AsDate::Date_setMilliseconds(Date* self) const
{
}

void AsDate::Date_setMinutes(Date* self) const
{
}

void AsDate::Date_setMonth(Date* self) const
{
}

void AsDate::Date_setSeconds(Date* self) const
{
}

void AsDate::Date_setTime(Date* self) const
{
}

void AsDate::Date_setUTCDate(Date* self) const
{
}

void AsDate::Date_setUTCFullYear(Date* self) const
{
}

void AsDate::Date_setUTCHours(Date* self) const
{
}

void AsDate::Date_setUTCMilliseconds(Date* self) const
{
}

void AsDate::Date_setUTCMinutes(Date* self) const
{
}

void AsDate::Date_setUTCMonth(Date* self) const
{
}

void AsDate::Date_setUTCSeconds(Date* self) const
{
}

void AsDate::Date_setYear(Date* self) const
{
}

std::string AsDate::Date_toString(Date* self) const
{
	return "[Date::toString not implemented]";
}

void AsDate::Date_UTC(Date* self) const
{
}

void AsDate::Date_valueOf(Date* self) const
{
}

	}
}
