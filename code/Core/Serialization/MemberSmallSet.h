/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallSet.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberArray.h"

namespace traktor
{

/*! Small set member. */
template < typename ValueType, typename ValueMember = Member< ValueType > >
class MemberSmallSet : public MemberArray
{
public:
	typedef SmallSet< ValueType > value_type;

	explicit MemberSmallSet(const wchar_t* const name, value_type& ref)
	:	MemberArray(name, nullptr)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}

	explicit MemberSmallSet(const wchar_t* const name, value_type& ref, const Attribute& attributes)
	:	MemberArray(name, &attributes)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}

	virtual void reserve(size_t size, size_t capacity) const override final
	{
		m_ref.clear();
	}

	virtual size_t size() const override final
	{
		return m_ref.size();
	}

	virtual void read(ISerializer& s) const override final
	{
		ValueType item;
		s >> ValueMember(L"item", item);
		m_ref.insert(item);
	}

	virtual void write(ISerializer& s) const override final
	{
		ValueType v = *m_iter++;
		s >> ValueMember(L"item", v);
	}

	virtual bool insert() const override final
	{
		m_ref.insert(ValueType());
		return true;
	}

private:
	value_type& m_ref;
	mutable typename value_type::iterator m_iter;
};

}

