/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_MemberStl_H
#define traktor_MemberStl_H

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

/*! \brief STL vector member. */
template < typename ValueType, typename ValueMember = Member< ValueType > >
class MemberStlVector : public MemberArray
{
public:
	typedef std::vector< ValueType > value_type;

	MemberStlVector(const wchar_t* const name, value_type& ref)
	:	MemberArray(name, 0)
	,	m_ref(ref)
	,	m_index(0)
	{
	}

	MemberStlVector(const wchar_t* const name, value_type& ref, const Attribute& attributes)
	:	MemberArray(name, &attributes)
	,	m_ref(ref)
	,	m_index(0)
	{
	}

	virtual void reserve(size_t size, size_t capacity) const T_OVERRIDE T_FINAL
	{
		m_ref.resize(size);
		m_ref.reserve(capacity);
	}

	virtual size_t size() const T_OVERRIDE T_FINAL
	{
		return m_ref.size();
	}
	
	virtual void read(ISerializer& s) const T_OVERRIDE T_FINAL
	{
		if (m_index < m_ref.size())
			s >> ValueMember(L"item", m_ref[m_index]);
		else
		{
			uint8_t zero[sizeof(ValueType)];
			std::memset(zero, 0, sizeof(zero));

			ValueType* item = new (zero) ValueType();
			if (!s.ensure(item != 0))
				return;

			s >> ValueMember(L"item", *item);

			m_ref.push_back(*item);
			item->~ValueType();
		}
		++m_index;
	}

	virtual void write(ISerializer& s) const T_OVERRIDE T_FINAL
	{
		s >> ValueMember(L"item", m_ref[m_index++]);
	}

	virtual bool insert() const T_OVERRIDE T_FINAL
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

/*! \brief STL list member. */
template < typename ValueType, typename ValueMember = Member< ValueType > >
class MemberStlList : public MemberArray
{
public:
	typedef std::list< ValueType > value_type;

	MemberStlList(const wchar_t* const name, value_type& ref)
	:	MemberArray(name, 0)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}
	
	MemberStlList(const wchar_t* const name, value_type& ref, const Attribute& attributes)
	:	MemberArray(name, &attributes)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}

	virtual void reserve(size_t size, size_t capacity) const T_OVERRIDE T_FINAL
	{
		m_ref.clear();
	}

	virtual size_t size() const T_OVERRIDE T_FINAL
	{
		return m_ref.size();
	}

	virtual void read(ISerializer& s) const T_OVERRIDE T_FINAL
	{
		m_ref.push_back(ValueType());
		s >> ValueMember(L"item", m_ref.back());
	}

	virtual void write(ISerializer& s) const T_OVERRIDE T_FINAL
	{
		s >> ValueMember(L"item", *m_iter++);
	}

	virtual bool insert() const T_OVERRIDE T_FINAL
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

/*! \brief STL set member. */
template < typename ValueType, typename ValueMember = Member< ValueType > >
class MemberStlSet : public MemberArray
{
public:
	typedef std::set< ValueType > value_type;

	MemberStlSet(const wchar_t* const name, value_type& ref)
	:	MemberArray(name, 0)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}

	MemberStlSet(const wchar_t* const name, value_type& ref, const Attribute& attributes)
	:	MemberArray(name, &attributes)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}
	
	virtual void reserve(size_t size, size_t capacity) const T_OVERRIDE T_FINAL
	{
		m_ref.clear();
	}

	virtual size_t size() const T_OVERRIDE T_FINAL
	{
		return m_ref.size();
	}

	virtual void read(ISerializer& s) const T_OVERRIDE T_FINAL
	{
		ValueType item;
		s >> ValueMember(L"item", item);
		m_ref.insert(item);
	}

	virtual void write(ISerializer& s) const T_OVERRIDE T_FINAL
	{
		ValueType v = *m_iter++;
		s >> ValueMember(L"item", v);
	}

	virtual bool insert() const T_OVERRIDE T_FINAL
	{
		m_ref.insert(ValueType());
		return true;
	}
	
private:
	value_type& m_ref;
	mutable typename value_type::iterator m_iter;
};

/*! \brief STL pair member. */
template < typename FirstType, typename SecondType, typename FirstMember = Member< FirstType >, typename SecondMember = Member< SecondType > >
class MemberStlPair : public MemberComplex
{
public:
	typedef std::pair< FirstType, SecondType > value_type;

	MemberStlPair(const wchar_t* const name, value_type& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const T_OVERRIDE T_FINAL
	{
		s >> FirstMember(L"first", m_ref.first);
		s >> SecondMember(L"second", m_ref.second);
	}

private:
	value_type& m_ref;
};

/*! \brief STL map member. */
template < typename KeyType, typename ValueType, typename PairMember = MemberStlPair< KeyType, ValueType > >
class MemberStlMap : public MemberArray
{
public:
	typedef std::map< KeyType, ValueType > value_type;

	MemberStlMap(const wchar_t* const name, value_type& ref)
	:	MemberArray(name, 0)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}

	MemberStlMap(const wchar_t* const name, value_type& ref, const Attribute& attributes)
	:	MemberArray(name, &attributes)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}

	virtual void reserve(size_t size, size_t capacity) const T_OVERRIDE T_FINAL
	{
		m_ref.clear();
	}

	virtual size_t size() const T_OVERRIDE T_FINAL
	{
		return m_ref.size();
	}

	virtual void read(ISerializer& s) const T_OVERRIDE T_FINAL
	{
		typename PairMember::value_type item;
		s >> PairMember(L"item", item);
		m_ref[item.first] = item.second;
	}

	virtual void write(ISerializer& s) const T_OVERRIDE T_FINAL
	{
		typename PairMember::value_type item = std::make_pair(m_iter->first, m_iter->second);
		s >> PairMember(L"item", item);
		++m_iter;
	}

	virtual bool insert() const T_OVERRIDE T_FINAL
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

#endif	// traktor_MemberStl_H
