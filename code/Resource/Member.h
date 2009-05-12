#ifndef traktor_resource_Member_H
#define traktor_resource_Member_H

#include "Core/Serialization/Serializable.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/Member.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RESOURCE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

/*! \brief Resource proxy serialization member.
 *
 * A resource proxy is serialized using it's
 * associated guid.
 */
template < typename Class, typename AssetClass = Class >
class Member : public MemberComplex
{
public:
	typedef Proxy< Class > value_type;

	Member(const std::wstring& name, value_type& ref)
	:	MemberComplex(name, false)
	,	m_ref(ref)
	{
	}
	
	virtual bool serialize(Serializer& s) const
	{
		Guid guid = m_ref.getGuid();
		if (!(s >> traktor::Member< Guid >(getName(), guid, &type_of< AssetClass >())))
			return false;
		m_ref = guid;
		return true;
	}
	
private:
	value_type& m_ref;
};

	}
}

#endif	// traktor_resource_Member_H
