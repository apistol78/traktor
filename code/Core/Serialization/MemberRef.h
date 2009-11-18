#ifndef traktor_MemberRef_H
#define traktor_MemberRef_H

#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{

/*! \brief Object reference member.
 * \ingroup Core
 */
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

	virtual bool serialize(ISerializer& s) const
	{
		Ref< ISerializable > object = m_ref;
		if (!(s >> Member< Ref< ISerializable > >(getName(), object, &type_of< Class >())))
			return false;
		m_ref = checked_type_cast< Class* >(object);
		return true;
	}
	
private:
	value_type& m_ref;
};

}

#endif	// traktor_MemberRef_H
