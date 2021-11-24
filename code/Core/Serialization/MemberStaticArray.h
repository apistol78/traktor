#pragma once

#include "Core/Serialization/MemberArray.h"

namespace traktor
{

/*! Static array member.
 * \ingroup Core
 */
template < typename ValueType, int ArraySize, typename ValueMember = Member< ValueType > >
class MemberStaticArray : public MemberArray
{
public:
	explicit MemberStaticArray(const wchar_t* const name, ValueType* arr, const wchar_t** elementNames = nullptr)
	:	MemberArray(name, nullptr)
	,	m_arr(arr)
	,	m_elementNames(elementNames)
	,	m_index(0)
	{
	}

	explicit MemberStaticArray(const wchar_t* const name, ValueType* arr, const Attribute& attributes)
	:	MemberArray(name, &attributes)
	,	m_arr(arr)
	,	m_elementNames(nullptr)
	,	m_index(0)
	{
	}

	virtual void reserve(size_t size, size_t capacity) const override final
	{
	}

	virtual size_t size() const override final
	{
		return ArraySize;
	}

	virtual void read(ISerializer& s) const override final
	{
		s >> ValueMember(
			m_elementNames ? m_elementNames[m_index] : L"item",
			m_arr[m_index]
		);
		++m_index;
	}

	virtual void write(ISerializer& s) const override final
	{
		s >> ValueMember(
			m_elementNames ? m_elementNames[m_index] : L"item",
			m_arr[m_index]
		);
		++m_index;
	}

	virtual bool insert() const override final
	{
		return false;
	}

private:
	ValueType* m_arr;
	const wchar_t** m_elementNames;
	mutable size_t m_index;
};

}

