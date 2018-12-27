/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_MemberRefArray_H
#define traktor_MemberRefArray_H

#include "Core/RefArray.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberInplaceRef.h"

namespace traktor
{

/*! \brief Array of objects member.
 * \ingroup Core
 */
template < typename Class >
class MemberRefArray : public MemberArray
{
public:
	typedef RefArray< Class > value_type;

	MemberRefArray(const wchar_t* const name, value_type& ref)
	:	MemberArray(name, &m_attribute)
	,	m_attribute(type_of< Class >())
	,	m_ref(ref)
	,	m_index(0)
	{
	}

	MemberRefArray(const wchar_t* const name, value_type& ref, const Attribute& attributes)
	:	MemberArray(name, &(m_attribute | attributes))
	,	m_attribute(type_of< Class >())
	,	m_ref(ref)
	,	m_index(0)
	{
	}

	virtual void reserve(size_t size, size_t capacity) const override final
	{
		m_ref.resize(size);
		m_ref.reserve(capacity);
	}

	virtual size_t size() const override final
	{
		return m_ref.size();
	}

	virtual void read(ISerializer& s) const override final
	{
		m_ref.resize(m_index + 1);
		s >> MemberInplaceRef< Class >(L"item", m_ref[m_index++]);
	}

	virtual void write(ISerializer& s) const override final
	{
		s >> MemberInplaceRef< Class >(L"item", m_ref[m_index++]);
	}

	virtual bool insert() const override final
	{
		return false;
	}

private:
	AttributeType m_attribute;
	value_type& m_ref;
	mutable size_t m_index;
};

}

#endif	// traktor_MemberRefArray_H
