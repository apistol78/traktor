#ifndef traktor_MemberSmallSet_H
#define traktor_MemberSmallSet_H

#include "Core/Containers/SmallSet.h"
#include "Core/Serialization/MemberArray.h"

namespace traktor
{

/*! \brief Small set member. */
template < typename ValueType, typename ValueMember = Member< ValueType > >
class MemberSmallSet : public MemberArray
{
public:
	typedef SmallSet< ValueType > value_type;

	MemberSmallSet(const wchar_t* const name, value_type& ref)
	:	MemberArray(name, 0)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}

	MemberSmallSet(const wchar_t* const name, value_type& ref, const Attribute& attributes)
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
		return false;
	}
	
private:
	value_type& m_ref;
	mutable typename value_type::iterator m_iter;
};

}

#endif	// traktor_MemberSmallSet_H
