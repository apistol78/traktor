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

	MemberAlignedVector(const std::wstring& name, value_type& ref)
	:	MemberArray(name)
	,	m_ref(ref)
	,	m_index(0)
	{
	}

	virtual void reserve(size_t size, size_t capacity) const
	{
		m_ref.resize(size);
		m_ref.reserve(size);
	}

	virtual size_t size() const
	{
		return m_ref.size();
	}
	
	virtual bool read(ISerializer& s) const
	{
		if (m_index < m_ref.size())
		{
			if (!(s >> ValueMember(L"item", m_ref[m_index])))
				return false;
		}
		else
		{
			ValueType item;
			if (!(s >> ValueMember(L"item", item)))
				return false;
			m_ref.push_back(item);
		}
		++m_index;
		return true;
	}

	virtual bool write(ISerializer& s) const
	{
		return s >> ValueMember(L"item", m_ref[m_index++]);
	}

	virtual bool insert() const
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
