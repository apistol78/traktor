#pragma once

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

/*! Object member type query predicate.
 * \ingroup Core
 */
class T_DLLCLASS RfpObjectType : public ReflectionMemberPredicate
{
	T_RTTI_CLASS;

public:
	RfpObjectType(const TypeInfo& objectType);

	virtual bool operator () (const ReflectionMember* member) const override final;

private:
	const TypeInfo& m_objectType;
};

}

