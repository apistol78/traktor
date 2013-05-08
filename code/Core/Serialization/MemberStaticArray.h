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
	MemberStaticArray(const wchar_t* const name, ValueType* arr, const wchar_t** elementNames = 0)
	:	MemberArray(name, 0)
	,	m_arr(arr)
	,	m_elementNames(elementNames)
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

	virtual void read(ISerializer& s) const
	{
		s >> ValueMember(
			m_elementNames ? m_elementNames[m_index] : L"item",
			m_arr[m_index]
		);
		++m_index;
	}

	virtual void write(ISerializer& s) const
	{
		s >> ValueMember(
			m_elementNames ? m_elementNames[m_index] : L"item",
			m_arr[m_index]
		);
		++m_index;
	}

	virtual bool insert() const
	{
		return false;
	}

private:
	ValueType* m_arr;
	const wchar_t** m_elementNames;
	mutable size_t m_index;
};

}

#endif	// traktor_MemberStaticArray_H
