/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Reflection/RfmObject.h"
#include "Core/Reflection/RfpObjectType.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.RfpObjectType", RfpObjectType, ReflectionMemberPredicate)

RfpObjectType::RfpObjectType(const TypeInfo& objectType)
:	m_objectType(objectType)
{
}

bool RfpObjectType::operator () (const ReflectionMember* member) const
{
	T_ASSERT(member);
	if (const RfmObject* objectMember = dynamic_type_cast< const RfmObject* >(member))
	{
		if (objectMember->get())
		{
			const TypeInfo& objectType = type_of(objectMember->get());
			if (is_type_of(m_objectType, objectType))
				return true;
		}
	}
	return false;
}

}
