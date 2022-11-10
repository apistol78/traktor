/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/InplaceRef.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

/*! Object reference member.
 * \ingroup Core
 */
template < typename Class >
class MemberInplaceRef : public MemberComplex
{
public:
	typedef InplaceRef< Class > value_type;

	explicit MemberInplaceRef(const wchar_t* const name, value_type ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		Ref< ISerializable > object = (ISerializable*)m_ref.ptr();
		s >> Member< ISerializable* >(
			getName(),
			object,
			AttributeType(type_of< Class >())
		);
		m_ref = checked_type_cast< Class* >(object.ptr());
	}

private:
	value_type m_ref;
};

}

