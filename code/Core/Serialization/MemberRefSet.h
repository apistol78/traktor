#ifndef traktor_MemberRefSet_H
#define traktor_MemberRefSet_H

#include "Core/RefSet.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{

/*! \brief Set of objects member.
 * \ingroup Core
 */
template < typename Class >
class MemberRefSet : public MemberArray
{
public:
	typedef RefSet< Class > value_type;

	MemberRefSet(const wchar_t* const name, value_type& ref)
	:	MemberArray(name, 0)
	,	m_ref(ref)
	,	m_iter(m_ref.begin())
	{
	}

	virtual void reserve(size_t size, size_t capacity) const T_OVERRIDE T_FINAL
	{
		m_ref.clear();
	}

	virtual size_t size() const T_OVERRIDE T_FINAL
	{
		return m_ref.size();
	}

	virtual void read(ISerializer& s) const T_OVERRIDE T_FINAL
	{
		Ref< Class > item;
		s >> MemberRef< Class >(L"item", item);
		m_ref.insert(item);
	}

	virtual void write(ISerializer& s) const T_OVERRIDE T_FINAL
	{
		Ref< Class > v = *m_iter++;
		s >> MemberRef< Class >(L"item", v);
	}

	virtual bool insert() const T_OVERRIDE T_FINAL
	{
		return false;
	}

private:
	value_type& m_ref;
	mutable typename value_type::const_iterator m_iter;
};

}

#endif	// traktor_MemberRefSet_H
