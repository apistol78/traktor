/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/OutputStream.h"
#include "Json/JsonArray.h"

namespace traktor::json
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.json.JsonArray", JsonArray, JsonNode)

void JsonArray::push(const Any& value)
{
	m_array.push_back(value);
}

bool JsonArray::write(OutputStream& os) const
{
	os << L"[" << Endl;
	os << IncreaseIndent;

	for (auto i = m_array.begin(); i != m_array.end(); ++i)
	{
		if (i != m_array.begin())
			os << L"," << Endl;

		switch (i->getType())
		{
		case Any::Type::Void:
			os << L"nil" << Endl;
			break;

		case Any::Type::Boolean:
			os << (i->getBooleanUnsafe() ? L"true" : L"false");
			break;

		case Any::Type::Int32:
			os << i->getInt32Unsafe();
			break;

		case Any::Type::Int64:
			os << i->getInt64Unsafe();
			break;

		case Any::Type::Float:
			os << i->getFloatUnsafe();
			break;

		case Any::Type::Double:
			os << i->getDoubleUnsafe();
			break;

		case Any::Type::String:
			os << L"\"" << i->getWideString() << L"\"";
			break;

		case Any::Type::Object:
			{
				if (auto node = dynamic_type_cast< const JsonNode* >(i->getObjectUnsafe()))
					node->write(os);
				else
					os << L"nil" << Endl;
			}
			break;
		}
	}
	if (!m_array.empty())
		os << Endl;

	os << DecreaseIndent;
	os << L"]" << Endl;
	return true;
}

}
