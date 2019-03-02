#pragma once

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

/*! \brief
 * \ingroup Core
 */
class T_DLLCLASS RuntimeException
{
public:
	RuntimeException(const std::wstring& what_);

	virtual const std::wstring& what() const throw();

private:
	std::wstring m_what;
};

}
