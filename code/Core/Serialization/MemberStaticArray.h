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
	MemberStaticArray(const std::wstring& name, ValueType* arr)
	:	MemberArray(name)
	,	m_arr(arr)
	,	m_last(&arr[0])
	{
	}

	virtual void reserve(size_t size) const
	{
	}

	virtual size_t size() const
	{
		return ArraySize;
	}

	virtual bool read(Serializer& s) const
	{
		return s >> ValueMember(L"item", *m_last++);
	}

	virtual bool write(Serializer& s, size_t index) const
	{
		return s >> ValueMember(L"item", m_arr[index]);
	}

	virtual bool insert() const
	{
		return false;
	}

private:
	ValueType* m_arr;
	mutable ValueType* m_last;
};

}

#endif	// traktor_MemberStaticArray_H
