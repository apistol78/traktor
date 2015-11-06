#ifndef traktor_RfpMemberType_H
#define traktor_RfpMemberType_H

#include "Core/Reflection/ReflectionMemberPredicate.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Member type query predicate.
 * \ingroup Core
 */
class T_DLLCLASS RfpMemberType : public ReflectionMemberPredicate
{
	T_RTTI_CLASS;

public:
	RfpMemberType(const TypeInfo& memberType);

	virtual bool operator () (const ReflectionMember* member) const T_OVERRIDE T_FINAL;

private:
	const TypeInfo& m_memberType;
};

}

#endif	// traktor_RfpMemberType_H
