#ifndef traktor_MemberAggregate_H
#define traktor_MemberAggregate_H

#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/Member.h"

namespace traktor
{

/*! \brief Aggregate member.
 * \ingroup Core
 */
template < typename Class >	
class MemberAggregate: public MemberComplex
{
public:
	typedef Class value_type;
	
	MemberAggregate(const std::wstring& name, value_type& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(Serializer& s) const
	{
		return s >> Member< Serializable >(getName(), m_ref, &type_of< Class >());
	}
	
private:
	value_type& m_ref;
};

}

#endif	// traktor_MemberAggregate_H
