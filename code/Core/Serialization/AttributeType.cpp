/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeType.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.AttributeType", AttributeType, Attribute)

AttributeType::AttributeType(const TypeInfo& memberType)
:	m_memberType(memberType)
{
}

const TypeInfo& AttributeType::getMemberType() const
{
	return m_memberType;
}

Ref< Attribute > AttributeType::internalClone() const
{
	return new AttributeType(m_memberType);
}

}
