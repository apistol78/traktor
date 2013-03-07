#ifndef traktor_resource_MemberIdProxy_H
#define traktor_resource_MemberIdProxy_H

#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComplex.h"
#include "Resource/IdProxy.h"

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
class MemberIdProxy : public MemberComplex
{
public:
	typedef IdProxy< Class > value_type;

	MemberIdProxy(const wchar_t* const name, value_type& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}
	
	virtual bool serialize(ISerializer& s) const
	{
		Guid id = m_ref.getId();
		Ref< IResourceHandle > handle = m_ref.getHandle();
		
		bool result = (s >> traktor::Member< traktor::Guid >(
			getName(),
			id,
			AttributeType(type_of< Class >())
		));
		if (!result)
			return false;

		m_ref = IdProxy< Class >(handle, id);
		return true;
	}
	
private:
	value_type& m_ref;
};

	}
}

#endif	// traktor_resource_MemberIdProxy_H
