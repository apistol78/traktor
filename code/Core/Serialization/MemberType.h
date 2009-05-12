#ifndef traktor_MemberType_H
#define traktor_MemberType_H

#include "Core/Serialization/MemberComplex.h"
#include "Core/Rtti/Type.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Rtti type member.
 * \ingroup Core
 */
class T_DLLCLASS MemberType : public MemberComplex
{
public:
	MemberType(const std::wstring& name, const Type*& type);

	virtual bool serialize(Serializer& s) const;

private:
	const Type*& m_type;
};

}

#endif	// traktor_MemberType_H
