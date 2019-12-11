#pragma once

#include "Core/Rtti/TypeInfo.h"
#include "Core/Serialization/MemberComplex.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Rtti type member.
 * \ingroup Core
 */
class T_DLLCLASS MemberType : public MemberComplex
{
public:
	MemberType(const wchar_t* const name, const TypeInfo*& type);

	virtual void serialize(ISerializer& s) const override final;

private:
	const TypeInfo*& m_type;
};

}

