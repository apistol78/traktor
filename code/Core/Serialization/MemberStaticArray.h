#ifndef traktor_MemberStaticArray_H
#define traktor_MemberStaticArray_H

#include "Core/Serialization/MemberArray.h"

namespace traktor
{

/*! \brief Static array member.
 * \ingroup Core
 */
template < typename ValueType, int ArraySize, typename ValueMember = Member< ValueType > >
class MemberStaticArray : public MemberArray
{
public:
	MemberStaticArray(const wchar_t* const name, ValueType* arr)
	:	MemberArray(name, 0)
	,	m_arr(arr)
	,	m_index(0)
	{
	}

	MemberStaticArray(const wchar_t* const name, ValueType* arr, const Attribute& attributes)
	:	MemberArray(name, &attributes)
	,	m_arr(arr)
	,	m_index(0)
	{
	}

	virtual void reserve(size_t size, size_t capacity) const
	{
	}

	virtual size_t size() const
	{
		return ArraySize;
	}

	virtual bool read(ISerializer& s) const
	{
		return s >> ValueMember(L"item", m_arr[m_index++]);
	}

	virtual bool write(ISerializer& s) const
	{
		return s >> ValueMember(L"item", m_arr[m_index++]);
	}

	virtual bool insert() const
	{
		return false;
	}

private:
	ValueType* m_arr;
	mutable size_t m_index;
};

}

#endif	// traktor_MemberStaticArray_H
