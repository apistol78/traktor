#ifndef traktor_MemberAggregate_H
#define traktor_MemberAggregate_H

#include "Core/Meta/Traits.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/Member.h"

namespace traktor
{

/*! \brief Aggregate member.
 * \ingroup Core
 */
template < typename Class >	
class MemberAggregate : public MemberComplex
{
public:
	typedef typename IsPointer< Class >::base_t class_type;
	typedef class_type* value_type;
	
	MemberAggregate(const wchar_t* const name, value_type& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const T_OVERRIDE T_FINAL
	{
		Ref< ISerializable > rf = m_ref;
		s >> Member< ISerializable* >(
			getName(),
			rf, 
			&type_of< class_type >()
		);
		m_ref = checked_type_cast< class_type* >(rf);
	}
	
private:
	value_type& m_ref;
};

}

#endif	// traktor_MemberAggregate_H
