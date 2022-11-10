/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{

/*! STL map member. */
template
<
	typename KeyType,
	typename ValueType,
	typename KeyMember = Member< KeyType >,
	typename ValueMember = Member< ValueType >
>
class MemberSmallMap : public MemberArray
{
public:
	typedef SmallMap< KeyType, ValueType > value_type;
	typedef MemberStlPair< KeyType, ValueType, KeyMember, ValueMember > pair_member;

	explicit MemberSmallMap(const wchar_t* const name, value_type& ref)
	:	MemberArray(name, nullptr)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}

	explicit MemberSmallMap(const wchar_t* const name, value_type& ref, const Attribute& attributes)
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
		typename pair_member::value_type item;
		s >> pair_member(L"item", item);
		m_ref[item.first] = item.second;
	}

	virtual void write(ISerializer& s) const override final
	{
		typename pair_member::value_type item = std::make_pair(m_iter->first, m_iter->second);
		s >> pair_member(L"item", item);
		++m_iter;
	}

	virtual bool insert() const override final
	{
		m_ref.insert(std::make_pair< KeyType, ValueType >(
			KeyType(),
			ValueType()
		));
		return true;
	}

private:
	value_type& m_ref;
	mutable typename value_type::iterator m_iter;
};

}

