#ifndef traktor_MemberType_H
#define traktor_MemberType_H

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

/*! \brief Rtti type member.
 * \ingroup Core
 */
class T_DLLCLASS MemberType : public MemberComplex
{
public:
	MemberType(const wchar_t* const name, const TypeInfo*& type);

	virtual void serialize(ISerializer& s) const;

private:
	const TypeInfo*& m_type;
};

}

#endif	// traktor_MemberType_H
