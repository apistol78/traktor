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
	{
	}

	virtual void reserve(size_t size) const
	{
		m_ref.resize(0);
		m_ref.reserve(size);
	}

	virtual size_t size() const
	{
		return m_ref.size();
	}
	
	virtual bool read(Serializer& s) const
	{
		ValueType item;
		if (!(s >> ValueMember(L"item", item)))
			return false;

		m_ref.push_back(item);
		return true;
	}

	virtual bool write(Serializer& s, size_t index) const
	{
		return s >> ValueMember(L"item", m_ref[index]);
	}

	virtual bool insert() const
	{
		m_ref.push_back(ValueType());
		return true;
	}
	
private:
	value_type& m_ref;
};

//@}

}

#endif	// traktor_MemberAlignedVector_H
