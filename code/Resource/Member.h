#ifndef traktor_resource_Member_H
#define traktor_resource_Member_H

#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComplex.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

/*! \brief Resource id serialization member.
 * \ingroup Resource
 */
template < typename Class >
class Member : public MemberComplex
{
public:
	typedef Id< Class > value_type;

	Member(const wchar_t* const name, value_type& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}
	
	virtual void serialize(ISerializer& s) const
	{
		Guid id = m_ref;
		s >> traktor::Member< traktor::Guid >(
			getName(),
			id,
			AttributeType(type_of< Class >())
		);
		m_ref = Id< Class >(id);
	}
	
private:
	value_type& m_ref;
};

	}
}

#endif	// traktor_resource_Member_H
