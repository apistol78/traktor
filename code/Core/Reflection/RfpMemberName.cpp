/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberName.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.RfpMemberName", RfpMemberName, ReflectionMemberPredicate)

RfpMemberName::RfpMemberName(const std::wstring& memberName)
:	m_memberName(memberName)
{
}

bool RfpMemberName::operator () (const ReflectionMember* member) const
{
	T_ASSERT(member);
	return member->getName() == m_memberName;
}

}
