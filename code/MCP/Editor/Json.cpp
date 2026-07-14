/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Editor/Json.h"

#include "Core/Class/Any.h"
#include "Core/Io/OutputStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Json/JsonArray.h"
#include "Json/JsonDocument.h"
#include "Json/JsonMember.h"
#include "Json/JsonObject.h"

namespace traktor::mcp
{
namespace
{

void writeEscaped(OutputStream& os, const std::wstring& str)
{
	const wchar_t* hex = L"0123456789abcdef";
	os << L"\"";
	for (wchar_t ch : str)
	{
		switch (ch)
		{
		case L'\"':
			os << L"\\\"";
			break;
		case L'\\':
			os << L"\\\\";
			break;
		case L'\b':
			os << L"\\b";
			break;
		case L'\f':
			os << L"\\f";
			break;
		case L'\n':
			os << L"\\n";
			break;
		case L'\r':
			os << L"\\r";
			break;
		case L'\t':
			os << L"\\t";
			break;
		default:
			if (ch < 0x20)
			{
				// Control character; emit as \u00XX escape.
				os << L"\\u00";
				os << hex[(ch >> 4) & 0xf];
				os << hex[ch & 0xf];
			}
			else
				os << ch;
			break;
		}
	}
	os << L"\"";
}

/*! Convert a value parsed by the json module into our model. */
Ref< Json > fromAny(const Any& any)
{
	switch (any.getType())
	{
	case Any::Type::Boolean:
		return Json::createBoolean(any.getBooleanUnsafe());

	case Any::Type::Int32:
		return Json::createNumber(any.getInt32Unsafe());

	case Any::Type::Int64:
		return Json::createNumber(any.getInt64Unsafe());

	case Any::Type::Float:
		return Json::createReal(any.getFloatUnsafe());

	case Any::Type::Double:
		return Json::createReal(any.getDoubleUnsafe());

	case Any::Type::String:
		return Json::createString(any.getWideString());

	case Any::Type::Object:
		{
			if (auto object = dynamic_type_cast< const json::JsonObject* >(any.getObjectUnsafe()))
			{
				Ref< Json > result = Json::createObject();
				for (auto member : object->get())
					result->set(member->getName(), fromAny(member->getValue()));
				return result;
			}
			else if (auto array = dynamic_type_cast< const json::JsonArray* >(any.getObjectUnsafe()))
			{
				Ref< Json > result = Json::createArray();
				for (const auto& value : array->get())
					result->push(fromAny(value));
				return result;
			}
			return Json::createNull();
		}

	default:
		return Json::createNull();
	}
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.Json", Json, Object)

Json::~Json()
{
}

Ref< Json > Json::parse(IStream* stream)
{
	json::JsonDocument document;
	if (!document.loadFromStream(stream))
		return nullptr;
	if (document.size() == 0)
		return nullptr;
	return fromAny(document.get(0));
}

Ref< Json > Json::parse(const std::wstring& text)
{
	json::JsonDocument document;
	if (!document.loadFromText(text))
		return nullptr;
	if (document.size() == 0)
		return nullptr;
	return fromAny(document.get(0));
}

Ref< Json > Json::createNull()
{
	Ref< Json > json = new Json();
	json->m_type = Type::Null;
	return json;
}

Ref< Json > Json::createBoolean(bool value)
{
	Ref< Json > json = new Json();
	json->m_type = Type::Boolean;
	json->m_boolean = value;
	return json;
}

Ref< Json > Json::createNumber(int64_t value)
{
	Ref< Json > json = new Json();
	json->m_type = Type::Number;
	json->m_number = value;
	return json;
}

Ref< Json > Json::createReal(double value)
{
	Ref< Json > json = new Json();
	json->m_type = Type::Real;
	json->m_real = value;
	return json;
}

Ref< Json > Json::createString(const std::wstring& value)
{
	Ref< Json > json = new Json();
	json->m_type = Type::String;
	json->m_string = value;
	return json;
}

Ref< Json > Json::createArray()
{
	Ref< Json > json = new Json();
	json->m_type = Type::Array;
	return json;
}

Ref< Json > Json::createObject()
{
	Ref< Json > json = new Json();
	json->m_type = Type::Object;
	return json;
}

bool Json::getBoolean(bool defaultValue) const
{
	if (m_type == Type::Boolean)
		return m_boolean;
	else if (m_type == Type::Number)
		return m_number != 0;
	else
		return defaultValue;
}

int64_t Json::getNumber(int64_t defaultValue) const
{
	if (m_type == Type::Number)
		return m_number;
	else if (m_type == Type::Real)
		return (int64_t)m_real;
	else
		return defaultValue;
}

double Json::getReal(double defaultValue) const
{
	if (m_type == Type::Real)
		return m_real;
	else if (m_type == Type::Number)
		return (double)m_number;
	else
		return defaultValue;
}

std::wstring Json::getString(const std::wstring& defaultValue) const
{
	return m_type == Type::String ? m_string : defaultValue;
}

Json* Json::set(const std::wstring& key, Json* value)
{
	for (size_t i = 0; i < m_memberNames.size(); ++i)
	{
		if (m_memberNames[i] == key)
		{
			m_memberValues[i] = value;
			return this;
		}
	}
	m_memberNames.push_back(key);
	m_memberValues.push_back(value);
	return this;
}

Json* Json::setBoolean(const std::wstring& key, bool value)
{
	return set(key, createBoolean(value));
}

Json* Json::setNumber(const std::wstring& key, int64_t value)
{
	return set(key, createNumber(value));
}

Json* Json::setString(const std::wstring& key, const std::wstring& value)
{
	return set(key, createString(value));
}

Json* Json::getMember(const std::wstring& key) const
{
	for (size_t i = 0; i < m_memberNames.size(); ++i)
		if (m_memberNames[i] == key)
			return m_memberValues[i];
	return nullptr;
}

uint32_t Json::getMemberCount() const
{
	return (uint32_t)m_memberNames.size();
}

const std::wstring& Json::getMemberName(uint32_t index) const
{
	return m_memberNames[index];
}

Json* Json::getMemberValue(uint32_t index) const
{
	return index < m_memberValues.size() ? m_memberValues[index] : nullptr;
}

Json* Json::push(Json* value)
{
	m_array.push_back(value);
	return this;
}

uint32_t Json::size() const
{
	return (uint32_t)m_array.size();
}

Json* Json::at(uint32_t index) const
{
	return index < m_array.size() ? m_array[index] : nullptr;
}

void Json::write(OutputStream& os) const
{
	switch (m_type)
	{
	case Type::Null:
		os << L"null";
		break;

	case Type::Boolean:
		os << (m_boolean ? L"true" : L"false");
		break;

	case Type::Number:
		os << m_number;
		break;

	case Type::Real:
		os << m_real;
		break;

	case Type::String:
		writeEscaped(os, m_string);
		break;

	case Type::Array:
		{
			os << L"[";
			for (uint32_t i = 0; i < m_array.size(); ++i)
			{
				if (i > 0)
					os << L",";
				if (m_array[i])
					m_array[i]->write(os);
				else
					os << L"null";
			}
			os << L"]";
		}
		break;

	case Type::Object:
		{
			os << L"{";
			for (size_t i = 0; i < m_memberNames.size(); ++i)
			{
				if (i > 0)
					os << L",";
				writeEscaped(os, m_memberNames[i]);
				os << L":";
				if (m_memberValues[i])
					m_memberValues[i]->write(os);
				else
					os << L"null";
			}
			os << L"}";
		}
		break;
	}
}

std::wstring Json::toString() const
{
	StringOutputStream ss;
	write(ss);
	return ss.str();
}

}
