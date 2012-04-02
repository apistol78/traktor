#ifndef traktor_MemberSmallMap_H
#define traktor_MemberSmallMap_H

#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{

/*! \brief STL map member. */
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

	MemberSmallMap(const wchar_t* const name, value_type& ref)
	:	MemberArray(name, 0)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}

	MemberSmallMap(const wchar_t* const name, value_type& ref, const Attribute& attributes)
	:	MemberArray(name, &attributes)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}

	virtual void reserve(size_t size, size_t capacity) const
	{
		m_ref.clear();
	}

	virtual size_t size() const
	{
		return m_ref.size();
	}

	virtual bool read(ISerializer& s) const
	{
		typename pair_member::value_type item;
		if (!(s >> pair_member(L"item", item)))
			return false;
		m_ref[item.first] = item.second;
		return true;
	}

	virtual bool write(ISerializer& s) const
	{
		typename pair_member::value_type item = std::make_pair(m_iter->first, m_iter->second);
		if (!(s >> pair_member(L"item", item)))
			return false;
		++m_iter;
		return true;
	}

	virtual bool insert() const
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

#endif	// traktor_MemberSmallMap_H
