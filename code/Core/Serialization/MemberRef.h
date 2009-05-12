#ifndef traktor_MemberRef_H
#define traktor_MemberRef_H

#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/Serializable.h"

namespace traktor
{

/*! \ingroup Core */
//@{

template < typename Class >	
class MemberRef : public MemberComplex
{
public:
	typedef Ref< Class > value_type;
	
	MemberRef(const std::wstring& name, value_type& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(Serializer& s) const
	{
		Class* object = m_ref.getPtr();
		if (!(s >> Member< Serializable* >(getName(), reinterpret_cast< Serializable*& >(object), &type_of< Class >())))
			return false;
		m_ref = object;
		return true;
	}
	
private:
	value_type& m_ref;
};

template < typename Class, typename MemberType = MemberRef< Class > >
class MemberRefArray : public MemberArray
{
public:
	typedef RefArray< Class > value_type;

	MemberRefArray(const std::wstring& name, value_type& ref)
	:	MemberArray(name)
	,	m_ref(ref)
	{
	}

	virtual const Type* getType() const
	{
		return &Class::getClassType();
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
		Ref< Class > object;
		if (!(s >> MemberType(L"item", object)))
			return false;

		m_ref.push_back(object);
		return true;
	}

	virtual bool write(Serializer& s, size_t index) const
	{
		Ref< Class > object = m_ref[index];
		return s >> MemberType(L"item", object);
	}

	virtual bool insert() const
	{
		return false;
	}

private:
	value_type& m_ref;
};

template < typename Class, typename MemberType = MemberRef< Class > >
class MemberRefList : public MemberArray
{
public:
	typedef RefList< Class > value_type;

	MemberRefList(const std::wstring& name, value_type& ref)
	:	MemberArray(name)
	,	m_ref(ref)
	{
	}

	virtual const Type* getType() const
	{
		return &Class::getClassType();
	}

	virtual void reserve(size_t size) const
	{
		m_ref.clear();
	}

	virtual size_t size() const
	{
		return m_ref.size();
	}

	virtual bool read(Serializer& s) const
	{
		Ref< Class > object;
		if (!(s >> MemberType(L"item", object)))
			return false;

		m_ref.push_back(object);
		return true;
	}

	virtual bool write(Serializer& s, size_t index) const
	{
		typename value_type::iterator i = m_ref.begin(); std::advance(i, index);
		Ref< Class > object = *i;
		return s >> MemberType(L"item", object);
	}

	virtual bool insert() const
	{
		return false;
	}

private:
	value_type& m_ref;
};

//@}

}

#endif	// traktor_MemberRef_H
