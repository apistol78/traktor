#pragma once

#include "Core/Containers/StaticVector.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

/*! \ingroup Core */
//@{

/*! \brief Static vector member. */
template < typename ValueType, int Capacity, typename ValueMember = Member< ValueType > >
class MemberStaticVector : public MemberArray
{
public:
	typedef StaticVector< ValueType, Capacity > value_type;

	MemberStaticVector(const wchar_t* const name, value_type& ref)
	:	MemberArray(name, 0)
	,	m_ref(ref)
	,	m_index(0)
	{
	}

	MemberStaticVector(const wchar_t* const name, value_type& ref, const Attribute& attributes)
	:	MemberArray(name, &attributes)
	,	m_ref(ref)
	,	m_index(0)
	{
	}

	virtual void reserve(size_t size, size_t capacity) const override final
	{
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
			ValueType item;
			s >> ValueMember(L"item", item);
			m_ref.push_back(item);
		}
		++m_index;
	}

	virtual void write(ISerializer& s) const override final
	{
		s >> ValueMember(L"item", m_ref[m_index++]);
	}

	virtual bool insert() const override final
	{
		m_ref.push_back(ValueType());
		return true;
	}
	
private:
	value_type& m_ref;
	mutable size_t m_index;
};

//@}

}
