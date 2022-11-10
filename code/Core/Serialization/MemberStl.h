/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cstring>
#include <list>
#include <map>
#include <set>
#include <vector>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

/*! \ingroup Core */
//@{

/*! STL vector member. */
template < typename ValueType, typename ValueMember = Member< ValueType > >
class MemberStlVector : public MemberArray
{
public:
	typedef std::vector< ValueType > value_type;

	explicit MemberStlVector(const wchar_t* const name, value_type& ref)
	:	MemberArray(name, nullptr)
	,	m_ref(ref)
	,	m_index(0)
	{
	}

	explicit MemberStlVector(const wchar_t* const name, value_type& ref, const Attribute& attributes)
	:	MemberArray(name, &attributes)
	,	m_ref(ref)
	,	m_index(0)
	{
	}

	virtual void reserve(size_t size, size_t capacity) const override final
	{
		m_ref.reserve(capacity);
		m_ref.resize(size);
	}

	virtual size_t size() const override final
	{
		return m_ref.size();
	}

	virtual void read(ISerializer& s) const override final
	{
		if (m_index < m_ref.size())
			s >> ValueMember(L"item", m_ref[m_index]);
		else
		{
			uint8_t zero[sizeof(ValueType)];
			std::memset(zero, 0, sizeof(zero));

			ValueType* item = new (zero) ValueType();
			if (!s.ensure(item != nullptr))
				return;

			s >> ValueMember(L"item", *item);

			m_ref.push_back(*item);
			item->~ValueType();
		}
		++m_index;
	}

	virtual void write(ISerializer& s) const override final
	{
		s >> ValueMember(L"item", m_ref[m_index++]);
	}

	virtual bool insert() const override final
	{
		uint8_t zero[sizeof(ValueType)];
		std::memset(zero, 0, sizeof(zero));

		ValueType* item = new (zero) ValueType();
		if (!item)
			return false;

		m_ref.push_back(*item);
		item->~ValueType();

		return true;
	}

private:
	value_type& m_ref;
	mutable size_t m_index;
};

/*! STL list member. */
template < typename ValueType, typename ValueMember = Member< ValueType > >
class MemberStlList : public MemberArray
{
public:
	typedef std::list< ValueType > value_type;

	explicit MemberStlList(const wchar_t* const name, value_type& ref)
	:	MemberArray(name, nullptr)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}

	explicit MemberStlList(const wchar_t* const name, value_type& ref, const Attribute& attributes)
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
		m_ref.push_back(ValueType());
		s >> ValueMember(L"item", m_ref.back());
	}

	virtual void write(ISerializer& s) const override final
	{
		s >> ValueMember(L"item", *m_iter++);
	}

	virtual bool insert() const override final
	{
		uint8_t value[sizeof(ValueType)];
		std::memset(value, 0, sizeof(value));

		ValueType* v = new (value) ValueType();
		m_ref.push_back(*v);
		v->~ValueType();

		return true;
	}

private:
	value_type& m_ref;
	mutable typename value_type::iterator m_iter;
};

/*! STL set member. */
template < typename ValueType, typename ValueMember = Member< ValueType > >
class MemberStlSet : public MemberArray
{
public:
	typedef std::set< ValueType > value_type;

	explicit MemberStlSet(const wchar_t* const name, value_type& ref)
	:	MemberArray(name, nullptr)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}

	explicit MemberStlSet(const wchar_t* const name, value_type& ref, const Attribute& attributes)
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

/*! STL pair member. */
template < typename FirstType, typename SecondType, typename FirstMember = Member< FirstType >, typename SecondMember = Member< SecondType > >
class MemberStlPair : public MemberComplex
{
public:
	typedef std::pair< FirstType, SecondType > value_type;

	explicit MemberStlPair(const wchar_t* const name, value_type& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	explicit MemberStlPair(const wchar_t* const name, value_type& ref, const Attribute& attributes)
	:	MemberComplex(name, true, attributes)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		s >> FirstMember(L"first", m_ref.first);
		s >> SecondMember(L"second", m_ref.second);
	}

private:
	value_type& m_ref;
};

/*! STL map member. */
template <
	typename KeyType,
	typename ValueType,
	typename KeyMember = Member< KeyType >,
	typename ValueMember = Member< ValueType >
>
class MemberStlMap : public MemberArray
{
public:
	typedef std::map< KeyType, ValueType > value_type;
	typedef MemberStlPair< KeyType, ValueType, KeyMember, ValueMember > pair_member;

	explicit MemberStlMap(const wchar_t* const name, value_type& ref)
	:	MemberArray(name, nullptr)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}

	explicit MemberStlMap(const wchar_t* const name, value_type& ref, const Attribute& attributes)
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

//@}

}

