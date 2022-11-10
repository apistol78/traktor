/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpMemberType.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.RfpMemberType", RfpMemberType, ReflectionMemberPredicate)

RfpMemberType::RfpMemberType(const TypeInfo& memberType)
:	m_memberType(memberType)
{
}

bool RfpMemberType::operator () (const ReflectionMember* member) const
{
	T_ASSERT(member);
	const TypeInfo& memberType = type_of(member);
	return is_type_of(m_memberType, memberType);
}

}
