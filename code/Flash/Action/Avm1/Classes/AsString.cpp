#include <limits>
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/String.h"
#include "Core/Misc/Split.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/String.h"
#include "Flash/Action/Classes/Array.h"
#include "Flash/Action/Avm1/Classes/AsString.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsString", AsString, ActionClass)

AsString::AsString()
:	ActionClass("String")
{
	Ref< ActionObject > prototype = new ActionObject();

	prototype->setMember("charAt", ActionValue(createNativeFunction(this, &AsString::String_charAt)));
	prototype->setMember("charCodeAt", ActionValue(createNativeFunction(this, &AsString::String_charCodeAt)));
	prototype->setMember("concat", ActionValue(createNativeFunction(this, &AsString::String_concat)));
	prototype->setMember("fromCharCode", ActionValue(createNativeFunction(this, &AsString::String_fromCharCode)));
	prototype->setMember("indexOf", ActionValue(createNativeFunction(this, &AsString::String_indexOf)));
	prototype->setMember("lastIndexOf", ActionValue(createNativeFunction(this, &AsString::String_lastIndexOf)));
	prototype->setMember("slice", ActionValue(createNativeFunction(this, &AsString::String_slice)));
	prototype->setMember("split", ActionValue(createNativeFunction(this, &AsString::String_split)));
	prototype->setMember("substr", ActionValue(createNativeFunction(this, &AsString::String_substr)));
	prototype->setMember("substring", ActionValue(createNativeFunction(this, &AsString::String_substring)));
	prototype->setMember("toLowerCase", ActionValue(createNativeFunction(this, &AsString::String_toLowerCase)));
	prototype->setMember("toString", ActionValue(createNativeFunction(this, &AsString::String_toString)));
	prototype->setMember("toUpperCase", ActionValue(createNativeFunction(this, &AsString::String_toUpperCase)));
	prototype->setMember("valueOf", ActionValue(createNativeFunction(this, &AsString::String_valueOf)));

	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

ActionValue AsString::construct(ActionContext* context, const ActionValueArray& args)
{
	if (args.size() > 0)
		return ActionValue(new String(args[0].getString()));
	else
		return ActionValue(new String());
}

void AsString::String_charAt(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);
	const std::string& st = self->get();

	uint32_t index = uint32_t(ca.args[0].getNumberSafe());

	if (index < st.length())
		ca.ret = ActionValue(new String(st[index]));
	else
		ca.ret = ActionValue(new String());
}

void AsString::String_charCodeAt(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);
	const std::string& st = self->get();

	uint32_t index = uint32_t(ca.args[0].getNumberSafe());

	if (index < st.length())
		ca.ret = ActionValue(avm_number_t(st[index]));
	else
		ca.ret = ActionValue(std::numeric_limits< avm_number_t >::signaling_NaN());
}

void AsString::String_concat(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);

	std::stringstream ss;
	ss << self->get();

	for (uint32_t i = 0; i < ca.args.size(); ++i)
		ss << ca.args[i].getStringSafe();

	ca.ret = ActionValue(ss.str());
}

void AsString::String_fromCharCode(CallArgs& ca)
{
	char charCode = char(ca.args[0].getNumberSafe());
	ca.ret = ActionValue(new String(charCode));
}

void AsString::String_indexOf(CallArgs& ca)
{
}

void AsString::String_lastIndexOf(CallArgs& ca)
{
}

void AsString::String_slice(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);
	const std::string& st = self->get();

	if (ca.args.size() >= 2)
	{
		uint32_t start = uint32_t(ca.args[0].getNumberSafe());
		uint32_t end = uint32_t(ca.args[1].getNumberSafe());
		if (start < st.length())
			ca.ret = ActionValue(st.substr(start, end - start));
		else
			ca.ret = ActionValue("");
	}
	else if (ca.args.size() >= 1)
	{
		uint32_t start = uint32_t(ca.args[0].getNumberSafe());
		if (start < st.length())
			ca.ret = ActionValue(st.substr(start));
		else
			ca.ret = ActionValue("");
	}
}

void AsString::String_split(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);
	const std::string& st = self->get();

	std::vector< std::string > words;
	if (ca.args.size() >= 2)
	{
		std::string delim = ca.args[0].getStringSafe();
		uint32_t limit = uint32_t(ca.args[1].getNumberSafe());
		Split< std::string >::word(st, delim, words, limit);
	}
	else if (ca.args.size() >= 1)
	{
		std::string delim = ca.args[0].getStringSafe();
		Split< std::string >::word(st, delim, words);
	}

	Ref< Array > arr = new Array();
	for (std::vector< std::string >::const_iterator i = words.begin(); i != words.end(); ++i)
		arr->push(ActionValue(*i));

	ca.ret = ActionValue(arr);
}

void AsString::String_substr(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);
	const std::string& st = self->get();

	uint32_t index = uint32_t(ca.args[0].getNumberSafe());
	uint32_t count = uint32_t(ca.args[1].getNumberSafe());

	if (index < st.length())
		ca.ret = ActionValue(st.substr(index, count));
	else
		ca.ret = ActionValue("");
}

void AsString::String_substring(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);
	const std::string& st = self->get();

	uint32_t start = uint32_t(ca.args[0].getNumberSafe());
	uint32_t end = uint32_t(ca.args[1].getNumberSafe());

	if (start < st.length())
		ca.ret = ActionValue(st.substr(start, end - start));
	else
		ca.ret = ActionValue("");
}

void AsString::String_toLowerCase(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);
	const std::string& st = self->get();
	ca.ret = ActionValue(new String(toLower(st)));
}

void AsString::String_toString(CallArgs& ca)
{
	ca.ret = ActionValue(ca.self);
}

void AsString::String_toUpperCase(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);
	const std::string& st = self->get();
	ca.ret = ActionValue(new String(toUpper(st)));
}

void AsString::String_valueOf(CallArgs& ca)
{
	Ref< String > self = checked_type_cast< String* >(ca.self);
	ca.ret = ActionValue(self->get());
}

	}
}
