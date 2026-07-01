/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include "Core/Class/Any.h"
#include "Core/Io/OutputStream.h"
#include "Json/JsonNode.h"

namespace traktor::json
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.json.JsonNode", JsonNode, Object)

void JsonNode::writeString(OutputStream& os, const std::wstring& str)
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

void JsonNode::writeValue(OutputStream& os, const Any& value)
{
	switch (value.getType())
	{
	case Any::Type::Void:
		os << L"null";
		break;

	case Any::Type::Boolean:
		os << (value.getBooleanUnsafe() ? L"true" : L"false");
		break;

	case Any::Type::Int32:
		os << value.getInt32Unsafe();
		break;

	case Any::Type::Int64:
		os << value.getInt64Unsafe();
		break;

	case Any::Type::Float:
		{
			// JSON has no representation for non-finite numbers; emit null.
			const float f = value.getFloatUnsafe();
			if (std::isfinite(f))
				os << f;
			else
				os << L"null";
		}
		break;

	case Any::Type::Double:
		{
			const double d = value.getDoubleUnsafe();
			if (std::isfinite(d))
				os << d;
			else
				os << L"null";
		}
		break;

	case Any::Type::String:
		writeString(os, value.getWideString());
		break;

	case Any::Type::Object:
		{
			if (auto node = dynamic_type_cast< const JsonNode* >(value.getObjectUnsafe()))
				node->write(os);
			else
				os << L"null";
		}
		break;
	}
}

}
