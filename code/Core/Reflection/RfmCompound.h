#pragma once

#include "Core/RefArray.h"
#include "Core/Reflection/ReflectionMember.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ReflectionMemberPredicate;

/*! \brief Reflected compound member.
 * \ingroup Core
 */
class T_DLLCLASS RfmCompound : public ReflectionMember
{
	T_RTTI_CLASS;

public:
	RfmCompound(const wchar_t* name);

	void addMember(ReflectionMember* member);

	void removeMember(ReflectionMember* member);

	uint32_t getMemberCount() const;

	ReflectionMember* getMember(uint32_t index) const;

	const RefArray< ReflectionMember >& getMembers() const;

	ReflectionMember* findMember(const ReflectionMemberPredicate& predicate) const;

	void findMembers(const ReflectionMemberPredicate& predicate, RefArray< ReflectionMember >& outMembers) const;

	template < typename MemberType >
	void findMembers(RefArray< MemberType >& outMembers) const
	{
		for (auto member : m_members)
		{
			if (is_a< MemberType >(member))
				outMembers.push_back(static_cast< MemberType* >(member));
		}
	}

	virtual bool replace(const ReflectionMember* source) override final;

private:
	RefArray< ReflectionMember > m_members;
};

}

