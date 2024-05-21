/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
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
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::resource
{

/*! Resource id serialization member.
 * \ingroup Resource
 */
template < typename Class >
class Member : public MemberComplex
{
public:
	typedef Id< Class > value_type;

	explicit Member(const wchar_t* const name, value_type& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	explicit Member(const wchar_t* const name, value_type& ref, const Attribute& attributes)
	:	MemberComplex(name, false, attributes)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		AttributeType attributeType(type_of< Class >());
		Guid id = m_ref;
		s >> traktor::Member< traktor::Guid >(
			getName(),
			id,
			getAttributes() ? attributeType | *getAttributes() : attributeType
		);
		m_ref = Id< Class >(id);
	}

private:
	value_type& m_ref;
};

/*! Obsolete member.
 * 
 * Instead of declaring dummy variables in each serialize method
 * use this wrapper instead.
 */
template < typename T >
class ObsoleteMember : public Member< T >
{
public:
	explicit ObsoleteMember(const wchar_t* const name)
	:	Member< T >(name, m_dummy)
	{
	}

	explicit ObsoleteMember(const wchar_t* const name, const Attribute& attributes)
	:	Member< T >(name, m_dummy, attributes)
	{
	}

private:
	typename Member< T >::value_type m_dummy;
};

}
