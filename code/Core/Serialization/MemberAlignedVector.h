/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_MemberAlignedVector_H
#define traktor_MemberAlignedVector_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

/*! \ingroup Core */
//@{

/*! \brief Aligned vector member. */
template < typename ValueType, typename ValueMember = Member< ValueType > >
class MemberAlignedVector : public MemberArray
{
public:
	typedef AlignedVector< ValueType > value_type;

	MemberAlignedVector(const wchar_t* const name, value_type& ref)
	:	MemberArray(name, 0)
	,	m_ref(ref)
	,	m_index(0)
	{
	}

	MemberAlignedVector(const wchar_t* const name, value_type& ref, const Attribute& attributes)
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
			ValueType item;
			s >> ValueMember(L"item", item);
			m_ref.push_back(item);
		}
		++m_index;
	}

	virtual void write(ISerializer& s) const T_OVERRIDE T_FINAL
	{
		s >> ValueMember(L"item", m_ref[m_index++]);
	}

	virtual bool insert() const T_OVERRIDE T_FINAL
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

#endif	// traktor_MemberAlignedVector_H
