#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ReflectionMember;

/*! Reflection member query predicate.
 * \ingroup Core
 */
class T_DLLCLASS ReflectionMemberPredicate : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool operator () (const ReflectionMember* member) const = 0;
};

}

