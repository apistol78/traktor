/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/OutputStream.h"
#include "Json/JsonMember.h"

namespace traktor::json
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.json.JsonMember", JsonMember, JsonNode)

JsonMember::JsonMember(const std::wstring& name, const Any& value)
:	m_name(name)
,	m_value(value)
{
}

bool JsonMember::write(OutputStream& os) const
{
	writeString(os, m_name);
	os << L": ";
	writeValue(os, m_value);
	return true;
}

}
