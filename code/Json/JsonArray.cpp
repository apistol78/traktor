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

		writeValue(os, *i);
	}
	if (!m_array.empty())
		os << Endl;

	os << DecreaseIndent;
	os << L"]" << Endl;
	return true;
}

}
