/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_RfpObjectType_H
#define traktor_RfpObjectType_H

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

/*! \brief Object member type query predicate.
 * \ingroup Core
 */
class T_DLLCLASS RfpObjectType : public ReflectionMemberPredicate
{
	T_RTTI_CLASS;

public:
	RfpObjectType(const TypeInfo& objectType);

	virtual bool operator () (const ReflectionMember* member) const T_OVERRIDE T_FINAL;

private:
	const TypeInfo& m_objectType;
};

}

#endif	// traktor_RfpObjectType_H
