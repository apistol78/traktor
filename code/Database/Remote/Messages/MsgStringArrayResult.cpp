/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Remote/Messages/MsgStringArrayResult.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.MsgStringArrayResult", 0, MsgStringArrayResult, IMessage)

MsgStringArrayResult::MsgStringArrayResult()
{
}

MsgStringArrayResult::MsgStringArrayResult(const std::vector< std::wstring >& values)
:	m_values(values)
{
}

void MsgStringArrayResult::add(const std::wstring& value)
{
	m_values.push_back(value);
}

uint32_t MsgStringArrayResult::count()
{
	return uint32_t(m_values.size());
}

const std::wstring& MsgStringArrayResult::get(uint32_t index) const
{
	return m_values[index];
}

void MsgStringArrayResult::serialize(ISerializer& s)
{
	s >> MemberStlVector< std::wstring >(L"handles", m_values);
}

	}
}
