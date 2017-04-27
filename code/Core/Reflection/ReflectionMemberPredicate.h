/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ReflectionMemberPredicate_H
#define traktor_ReflectionMemberPredicate_H

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

/*! \brief Reflection member query predicate.
 * \ingroup Core
 */
class T_DLLCLASS ReflectionMemberPredicate : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool operator () (const ReflectionMember* member) const = 0;
};

}

#endif	// traktor_ReflectionMemberPredicate_H
