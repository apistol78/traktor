#include <limits>
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/String.h"
#include "Core/Misc/Split.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Common/String.h"
#include "Flash/Action/Common/Array.h"
#include "Flash/Action/Common/Classes/AsString.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsString", AsString, ActionClass)

AsString::AsString(ActionContext* context)
:	ActionClass(context, "String")
{
	setMember("fromCharCode", ActionValue(createNativeFunction(context, this, &AsString::String_fromCharCode)));

	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("charAt", ActionValue(createNativeFunction(context, this, &AsString::String_charAt)));
	prototype->setMember("charCodeAt", ActionValue(createNativeFunction(context, this, &AsString::String_charCodeAt)));
	prototype->setMember("concat", ActionValue(createNativeFunction(context, this, &AsString::String_concat)));
	prototype->setMember("indexOf", ActionValue(createNativeFunction(context, this, &AsString::String_indexOf)));
	prototype->setMember("lastIndexOf", ActionValue(createNativeFunction(context, this, &AsString::String_lastIndexOf)));
	prototype->setMember("slice", ActionValue(createNativeFunction(context, this, &AsString::String_slice)));
	prototype->setMember("split", ActionValue(createNativeFunction(context, this, &AsString::String_split)));
	prototype->setMember("substr", ActionValue(createNativeFunction(context, this, &AsString::String_substr)));
	prototype->setMember("substring", ActionValue(createNativeFunction(context, this, &AsString::String_substring)));
	prototype->setMember("toLowerCase", ActionValue(createNativeFunction(context, this, &AsString::String_toLowerCase)));
	prototype->setMember("toString", ActionValue(createNativeFunction(context, this, &AsString::String_toString)));
	prototype->setMember("toUpperCase", ActionValue(createNativeFunction(context, this, &AsString::String_toUpperCase)));
	prototype->setMember("valueOf", ActionValue(createNativeFunction(context, this, &AsString::String_valueOf)));

	prototype->setMember("constructor", ActionValue(this));
	setMember("prototype", ActionValue(prototype));
}

void AsString::initialize(ActionObject* self)
{
	self->addProperty("length", createNativeFunction(getContext(), this, &AsString::String_get_length), 0);
}

void AsString::construct(ActionObject* self, const ActionValueArray& args)
{
	Ref< String > s;

	if (args.size() > 0)
		s = new String(args[0].getString());
	else
		s = new String();

	self->setRelay(s);
}

ActionValue AsString::xplicit(const ActionValueArray& args)
{
	if (args.size() > 0)
		return args[0].toString();
	else
		return ActionValue("");
}

void AsString::String_fromCharCode(CallArgs& ca)
{
	char charCode[] = { char(ca.args[0].getInteger()), 0 };
	ca.ret = ActionValue(charCode);
}

std::string AsString::String_charAt(const String* self, uint32_t index) const
{
	char tmp[2] = { 0, 0 };
	const std::string& st = self->get();
	if (index < st.length())
		tmp[0] = st[index];
	return tmp;
}

uint32_t AsString::String_charCodeAt(const String* self, uint32_t index) const
{
	const std::string& st = self->get();
	if (index < st.length())
		return st[index];
	else
		return 0;
}

void AsString::String_concat(CallArgs& ca)
{
	Ref< String > self = ca.self->getRelay< String >();
	if (self)
	{
		std::stringstream ss;
		ss << self->get();

		for (uint32_t i = 0; i < ca.args.size(); ++i)
			ss << ca.args[i].getString();

		ca.ret = ActionValue(ss.str());
	}
}

int32_t AsString::String_indexOf(const String* self, const std::string& needle) const
{
	const std::string& st = self->get();
	size_t pos = st.find(needle);
	return pos != st.npos ? int32_t(pos) : -1;
}

int32_t AsString::String_lastIndexOf(const String* self, const std::string& needle) const
{
	const std::string& st = self->get();
	size_t pos = st.rfind(needle);
	return pos != st.npos ? int32_t(pos) : -1;
}

void AsString::String_slice(CallArgs& ca)
{
	Ref< String > self = ca.self->getRelay< String >();
	const std::string& st = self->get();

	if (ca.args.size() >= 2)
	{
		uint32_t start = uint32_t(ca.args[0].getInteger());
		uint32_t end = uint32_t(ca.args[1].getInteger());
		if (start < st.length())
			ca.ret = ActionValue(st.substr(start, end - start));
		else
			ca.ret = ActionValue("");
	}
	else if (ca.args.size() >= 1)
	{
		uint32_t start = uint32_t(ca.args[0].getInteger());
		if (start < st.length())
			ca.ret = ActionValue(st.substr(start));
		else
			ca.ret = ActionValue("");
	}
}

void AsString::String_split(CallArgs& ca)
{
	Ref< String > self = ca.self->getRelay< String >();
	const std::string& st = self->get();

	std::vector< std::string > words;
	if (ca.args.size() >= 2)
	{
		std::string delim = ca.args[0].getString();
		if (!delim.empty())
		{
			uint32_t limit = uint32_t(ca.args[1].getInteger());
			words.reserve(limit);

			Split< std::string >::any(st, delim, words, true, limit);
		}
	}
	else if (ca.args.size() >= 1)
	{
		std::string delim = ca.args[0].getString();
		if (!delim.empty())
			Split< std::string >::any(st, delim, words, true);
	}
	else
	{
		words.push_back(st);
	}

	Ref< Array > arr = new Array(uint32_t(words.size()));
	for (std::vector< std::string >::const_iterator i = words.begin(); i != words.end(); ++i)
		arr->push(ActionValue(*i));

	ca.ret = ActionValue(arr->getAsObject(ca.context));
}

void AsString::String_substr(CallArgs& ca)
{
	Ref< String > self = ca.self->getRelay< String >();
	const std::string& st = self->get();

	int32_t index = int32_t(ca.args[0].getInteger());
	if (index < 0)
	{
		index = int32_t(st.length()) + index;
		if (index < 0)
			index = 0;
	}

	if (ca.args.size() >= 2)
	{
		int32_t count = int32_t(ca.args[1].getInteger());
		if (index < st.length())
			ca.ret = ActionValue(st.substr(index, count));
		else
			ca.ret = ActionValue("");
	}
	else
	{
		if (index < st.length())
			ca.ret = ActionValue(st.substr(index));
		else
			ca.ret = ActionValue("");
	}
}

void AsString::String_substring(CallArgs& ca)
{
	Ref< String > self = ca.self->getRelay< String >();
	const std::string& st = self->get();

	uint32_t start = uint32_t(ca.args[0].getInteger());
	if (ca.args.size() >= 2)
	{
		uint32_t end = uint32_t(ca.args[1].getInteger());
		if (start < st.length())
			ca.ret = ActionValue(st.substr(start, end - start));
		else
			ca.ret = ActionValue("");
	}
	else
	{
		if (start < st.length())
			ca.ret = ActionValue(st.substr(start));
		else
			ca.ret = ActionValue("");
	}
}

void AsString::String_toLowerCase(CallArgs& ca)
{
	Ref< String > self = ca.self->getRelay< String >();
	const std::string& st = self->get();
	ca.ret = ActionValue(toLower(st));
}

void AsString::String_toString(CallArgs& ca)
{
	Ref< String > self = ca.self->getRelay< String >();
	if (self)
		ca.ret = ActionValue(self->get());
	else
		ca.ret = ActionValue("[object Object]");
}

void AsString::String_toUpperCase(CallArgs& ca)
{
	Ref< String > self = ca.self->getRelay< String >();
	const std::string& st = self->get();
	ca.ret = ActionValue(toUpper(st));
}

void AsString::String_valueOf(CallArgs& ca)
{
	Ref< String > self = ca.self->getRelay< String >();
	ca.ret = ActionValue(self->get());
}

int32_t AsString::String_get_length(const String* self) const
{
	return int32_t(self->get().length());
}

	}
}
