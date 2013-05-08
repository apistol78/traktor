#ifndef traktor_MemberComplex_H
#define traktor_MemberComplex_H

#include <string>
#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializer;

/*! \brief Complex member base.
 * \ingroup Core
 */
class T_DLLCLASS MemberComplex
{
public:
	MemberComplex(const wchar_t* const name, bool compound);

	virtual ~MemberComplex();

	const wchar_t* const getName() const;
	
	bool getCompound() const;

	virtual void serialize(ISerializer& s) const = 0;
	
private:
	const wchar_t* const m_name;
	bool m_compound;
};
	
}

#endif	// traktor_MemberComplex_H
