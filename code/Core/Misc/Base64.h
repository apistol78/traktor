#ifndef traktor_Base64_H
#define traktor_Base64_H

#include <string>
#include <vector>
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

/*! \brief Base64 encode binary data.
 * \ingroup Core
 */
class T_DLLCLASS Base64 : public Object
{
	T_RTTI_CLASS;

public:
	std::wstring encode(const uint8_t* data, uint32_t size, bool insertCrLf = false) const;

	std::wstring encode(const std::vector< uint8_t >& data, bool insertCrLf = false) const;

	std::vector< uint8_t > decode(const std::wstring& b64) const;
};

}

#endif	// traktor_Base64_H
