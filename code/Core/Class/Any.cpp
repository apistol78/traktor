/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Class/Any.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace
	{

char* refStringCreate(const char* s)
{
	size_t len = strlen(s);

	void* ptr = getAllocator()->alloc(sizeof(uint16_t) + (len + 1) * sizeof(char), 4, T_FILE_LINE);
	if (!ptr)
		return nullptr;

	uint16_t* base = static_cast< uint16_t* >(ptr);
	*base = 1;

	char* c = reinterpret_cast< char* >(base + 1);
	if (len > 0)
		std::memcpy(c, s, len * sizeof(char));

	c[len] = L'\0';
	return c;
}

char* refStringInc(char* s)
{
	uint16_t* base = reinterpret_cast< uint16_t* >(s) - 1;
	(*base)++;
	return s;
}

char* refStringDec(char* s)
{
	uint16_t* base = reinterpret_cast< uint16_t* >(s) - 1;
	if (--*base == 0)
	{
		getAllocator()->free(base);
		return nullptr;
	}
	return s;
}

	}

Any::Any()
:	m_type(Type::Void)
{
}

Any::Any(const Any& src)
:	m_type(src.m_type)
{
	if (m_type == Type::String)
		m_data.m_string = refStringInc(src.m_data.m_string);
	else if (m_type == Type::Object)
	{
		T_SAFE_ADDREF(src.m_data.m_object);
		m_data.m_object = src.m_data.m_object;
	}
	else
		m_data = src.m_data;
}

Any::Any(Any&& src) noexcept
:	m_type(src.m_type)
,	m_data(src.m_data)
{
	src.m_type = Type::Void;
}

Any::~Any()
{
	T_EXCEPTION_GUARD_BEGIN

	if (m_type == Type::String)
		refStringDec(m_data.m_string);
	else if (m_type == Type::Object)
		T_SAFE_RELEASE(m_data.m_object);

	T_EXCEPTION_GUARD_END
}

Any Any::fromBoolean(bool value)
{
	Any any;
	any.m_type = Type::Boolean;
	any.m_data.m_boolean = value;
	return any;
}

Any Any::fromInt32(int32_t value)
{
	Any any;
	any.m_type = Type::Int32;
	any.m_data.m_int32 = value;
	return any;
}

Any Any::fromInt64(int64_t value)
{
	Any any;
	any.m_type = Type::Int64;
	any.m_data.m_int64 = value;
	return any;
}

Any Any::fromFloat(float value)
{
	Any any;
	any.m_type = Type::Float;
	any.m_data.m_float = value;
	return any;
}

Any Any::fromString(const char* value)
{
	Any any;
	any.m_type = Type::String;
	any.m_data.m_string = refStringCreate(value);
	return any;
}

Any Any::fromString(const std::string& value)
{
	Any any;
	any.m_type = Type::String;
	any.m_data.m_string = refStringCreate(value.c_str());
	return any;
}

Any Any::fromString(const wchar_t* value)
{
	Any any;
	any.m_type = Type::String;
	any.m_data.m_string = refStringCreate(wstombs(Utf8Encoding(), value).c_str());
	return any;
}

Any Any::fromString(const std::wstring& value)
{
	Any any;
	any.m_type = Type::String;
	any.m_data.m_string = refStringCreate(wstombs(Utf8Encoding(), value).c_str());
	return any;
}

Any Any::fromObject(ITypedObject* value)
{
	T_SAFE_ANONYMOUS_ADDREF(value);
	Any any;
	any.m_type = Type::Object;
	any.m_data.m_object = value;
	return any;
}

bool Any::getBoolean() const
{
	switch (m_type)
	{
	case Type::Boolean:
		return m_data.m_boolean;
	case Type::Int32:
		return m_data.m_int32 != 0;
	case Type::Int64:
		return m_data.m_int64 != 0;
	case Type::Float:
		return m_data.m_float != 0.0f;
	case Type::String:
		return parseString< int32_t >(m_data.m_string) != 0;
	case Type::Object:
		return m_data.m_object != nullptr;
	default:
		break;
	}
	return false;
}

int32_t Any::getInt32() const
{
	switch (m_type)
	{
	case Type::Boolean:
		return m_data.m_boolean ? 1 : 0;
	case Type::Int32:
		return m_data.m_int32;
	case Type::Int64:
		return int32_t(m_data.m_int64);
	case Type::Float:
		return int32_t(m_data.m_float);
	case Type::String:
		return parseString< int32_t >(m_data.m_string);
	default:
		break;
	}
	return 0;
}

int64_t Any::getInt64() const
{
	switch (m_type)
	{
	case Type::Boolean:
		return m_data.m_boolean ? 1 : 0;
	case Type::Int32:
		return m_data.m_int32;
	case Type::Int64:
		return m_data.m_int64;
	case Type::Float:
		return int64_t(m_data.m_float);
	case Type::String:
		return parseString< int64_t >(m_data.m_string);
	default:
		break;
	}
	return 0;
}

float Any::getFloat() const
{
	switch (m_type)
	{
	case Type::Boolean:
		return m_data.m_boolean ? 1.0f : 0.0f;
	case Type::Int32:
		return float(m_data.m_int32);
	case Type::Int64:
		return float(m_data.m_int64);
	case Type::Float:
		return m_data.m_float;
	case Type::String:
		return parseString< float >(m_data.m_string);
	default:
		break;
	}
	return 0.0f;
}

std::string Any::getString() const
{
	switch (m_type)
	{
	case Type::Boolean:
		return m_data.m_boolean ? "true" : "false";
	case Type::Int32:
		return wstombs(Utf8Encoding(), toString(m_data.m_int32));
	case Type::Int64:
		return wstombs(Utf8Encoding(), toString(m_data.m_int64));
	case Type::Float:
		return wstombs(Utf8Encoding(), toString(m_data.m_float));
	case Type::String:
		return m_data.m_string;
	default:
		break;
	}
	return "";
}

std::wstring Any::getWideString() const
{
	switch (m_type)
	{
	case Type::Boolean:
		return m_data.m_boolean ? L"true" : L"false";
	case Type::Int32:
		return toString(m_data.m_int32);
	case Type::Int64:
		return toString(m_data.m_int64);
	case Type::Float:
		return toString(m_data.m_float);
	case Type::String:
		return mbstows(Utf8Encoding(), m_data.m_string);
	default:
		break;
	}
	return L"";
}

Any& Any::operator = (const Any& src)
{
	if (m_type == Type::String)
		refStringDec(m_data.m_string);
	else if (m_type == Type::Object)
		T_SAFE_RELEASE(m_data.m_object);

	m_type = src.m_type;
	m_data = src.m_data;

	if (m_type == Type::String)
		refStringInc(m_data.m_string);
	else if (m_type == Type::Object)
		T_SAFE_ADDREF(m_data.m_object);

	return *this;
}

Any& Any::operator = (Any&& src) noexcept
{
	if (m_type == Type::String)
	{
		if (src.m_type != Type::String || m_data.m_string != src.m_data.m_string)
			refStringDec(m_data.m_string);
	}
	else if (m_type == Type::Object)
	{
		if (src.m_type != Type::Object || m_data.m_object != src.m_data.m_object)
			T_SAFE_RELEASE(m_data.m_object);
	}

	m_type = src.m_type;
	m_data = src.m_data;

	src.m_type = Type::Void;
	return *this;
}

std::wstring Any::format() const
{
	StringOutputStream ss;

	ss << L"[";

	switch (m_type)
	{
	case Type::Boolean:
		ss << L"bool";
		break;

	case Type::Int32:
		ss << L"int32";
		break;

	case Type::Int64:
		ss << L"int64";
		break;

	case Type::Float:
		ss << L"float";
		break;

	case Type::String:
		ss << L"string";
		break;

	case Type::Object:
		ss << L"object";
		break;

	default:
		ss << L"void";
		break;
	}

	if (!isVoid())
	{
		ss << L";";
		if (!isObject())
			ss << getWideString();
		else
			ss << type_name(getObjectUnsafe());
	}

	ss << L"]";
	return ss.str();
}

}
