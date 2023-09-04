/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

/*! Object reference member.
 * \ingroup Core
 */
template < typename Class >
class MemberRef : public MemberComplex
{
public:
	typedef Ref< Class > value_type;

	explicit MemberRef(const wchar_t* const name, value_type& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	explicit MemberRef(const wchar_t* const name, value_type& ref, const Attribute& attributes)
	:	MemberComplex(name, false, attributes)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		AttributeType attributeType(type_of< Class >());
		Ref< ISerializable > object = (ISerializable*)m_ref.ptr();
		s >> Member< ISerializable* >(
			getName(),
			object,
			getAttributes() ? attributeType | *getAttributes() : attributeType
		);
		m_ref = checked_type_cast< Class* >(object);
	}

private:
	value_type& m_ref;
};

}

