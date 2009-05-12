#ifndef traktor_MemberComplex_H
#define traktor_MemberComplex_H

#include <string>
#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Serializer;

/*! \brief Complex member base.
 * \ingroup Core
 */
class T_DLLCLASS MemberComplex
{
public:
	MemberComplex(const std::wstring& name, bool compound);

	virtual ~MemberComplex();

	const std::wstring& getName() const;
	
	bool getCompound() const;

	virtual bool serialize(Serializer& s) const = 0;
	
private:
	std::wstring m_name;
	bool m_compound;
};
	
}

#endif	// traktor_MemberComplex_H
