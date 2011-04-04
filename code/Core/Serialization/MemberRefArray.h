#ifndef traktor_MemberRefArray_H
#define traktor_MemberRefArray_H

#include "Core/RefArray.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{

/*! \brief Array of objects member.
 * \ingroup Core
 */
template < typename Class, typename MemberType = MemberRef< Class > >
class MemberRefArray : public MemberArray
{
public:
	typedef RefArray< Class > value_type;

	MemberRefArray(const std::wstring& name, value_type& ref)
	:	MemberArray(name)
	,	m_ref(ref)
	,	m_index(0)
	{
	}

	virtual const TypeInfo* getType() const
	{
		return &Class::getClassTypeInfo();
	}

	virtual void reserve(size_t size, size_t capacity) const
	{
		m_ref.resize(size);
		m_ref.reserve(capacity);
	}

	virtual size_t size() const
	{
		return m_ref.size();
	}

	virtual bool read(ISerializer& s) const
	{
		Ref< Class > object;
		if (!(s >> MemberType(L"item", object)))
			return false;

		if (m_index < m_ref.size())
			m_ref[m_index] = object;
		else
			m_ref.push_back(object);

		++m_index;
		return true;
	}

	virtual bool write(ISerializer& s) const
	{
		Ref< Class > object = m_ref[m_index++];
		return s >> MemberType(L"item", object);
	}

	virtual bool insert() const
	{
		return false;
	}

private:
	value_type& m_ref;
	mutable size_t m_index;
};

}

#endif	// traktor_MemberRefArray_H
