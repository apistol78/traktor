#ifndef traktor_MemberRefArray_H
#define traktor_MemberRefArray_H

#include "Core/Heap/RefArray.h"
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
	,	m_value(ref)
	,	m_modified(false)
	{
	}

	virtual ~MemberRefArray()
	{
		if (m_modified)
			m_ref.swap(m_value);
	}

	virtual const Type* getType() const
	{
		return &Class::getClassType();
	}

	virtual void reserve(size_t size) const
	{
		m_value.resize(0);
		m_value.reserve(size);
		m_modified = true;
	}

	virtual size_t size() const
	{
		return m_value.size();
	}

	virtual bool read(Serializer& s) const
	{
		Ref< Class > object;
		if (!(s >> MemberType(L"item", object)))
			return false;

		m_value.push_back(checked_type_cast< Class* >(object));
		m_modified = true;

		return true;
	}

	virtual bool write(Serializer& s, size_t index) const
	{
		if (index >= m_value.size())
			T_FATAL_ERROR;
		Ref< Class > object = m_value[index];
		return s >> MemberType(L"item", object);
	}

	virtual bool insert() const
	{
		return false;
	}

private:
	value_type& m_ref;
	mutable value_type m_value;
	mutable bool m_modified;
};

}

#endif	// traktor_MemberRefArray_H
