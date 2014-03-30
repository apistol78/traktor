#ifndef traktor_RfmCompound_H
#define traktor_RfmCompound_H

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

	virtual bool replace(const ReflectionMember* source);

private:
	RefArray< ReflectionMember > m_members;
};

}

#endif	// traktor_RfmCompound_H
