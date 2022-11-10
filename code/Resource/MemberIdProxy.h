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
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComplex.h"
#include "Resource/IdProxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

/*! Resource id serialization member.
 * \ingroup Resource
 */
template < typename Class >
class MemberIdProxy : public MemberComplex
{
public:
	typedef IdProxy< Class > value_type;

	explicit MemberIdProxy(const wchar_t* const name, value_type& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		Guid id = m_ref.getId();
		Ref< ResourceHandle > handle = m_ref.getHandle();
		s >> traktor::Member< traktor::Guid >(
			getName(),
			id,
			AttributeType(type_of< Class >())
		);
		m_ref = IdProxy< Class >(handle, id);
	}

private:
	value_type& m_ref;
};

	}
}

