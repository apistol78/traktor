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

class T_DLLCLASS RfpObjectType : public ReflectionMemberPredicate
{
	T_RTTI_CLASS;

public:
	RfpObjectType(const TypeInfo& objectType);

	virtual bool operator () (const ReflectionMember* member) const;

private:
	const TypeInfo& m_objectType;
};

}

#endif	// traktor_RfpObjectType_H
