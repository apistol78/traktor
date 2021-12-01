#pragma once

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

/*! Character encoding.
 * \ingroup Core
 */
class T_DLLCLASS IEncoding : public Object
{
	T_RTTI_CLASS;

public:
	enum { MaxEncodingSize = 8 };

	/*! Translate from Unicode to special encoding.
	 *
	 * \param chars Unicode source characters.
	 * \param count Number of source characters.
	 * \param out Encoded characters, should be at least count * MaxEncodingSize bytes to ensure enough space.
	 * \return Number of bytes used in out array, -1 if encoding failed.
	 */
	virtual int32_t translate(const wchar_t* chars, int32_t count, uint8_t* out) const = 0;

	/*! Translate from special encoding to Unicode.
	 *
	 * \param in Input character in special encoding.
	 * \param count Number of valid bytes in input buffer.
	 * \param out Output unicode character.
	 * \return -1 if decoding failed, otherwise number of bytes "consumed" from input buffer.
	 */
	virtual int32_t translate(const uint8_t in[MaxEncodingSize], int32_t count, wchar_t& out) const = 0;
};

}

