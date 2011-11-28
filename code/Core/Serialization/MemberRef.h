#ifndef traktor_MemberRef_H
#define traktor_MemberRef_H

#include "Core/Ref.h"
#include "Core/Serialization/AttributeType.h"
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
	
	MemberRef(const wchar_t* const name, value_type& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		ISerializable* object = const_cast< ISerializable* >(static_cast< const ISerializable* >(m_ref.ptr()));
		if (!(s >> Member< ISerializable* >(
			getName(),
			object,
			AttributeType(type_of< Class >())
		)))
			return false;
		m_ref = checked_type_cast< Class* >(object);
		return true;
	}
	
private:
	value_type& m_ref;
};

}

#endif	// traktor_MemberRef_H
