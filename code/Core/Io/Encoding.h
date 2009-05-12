#ifndef traktor_Encoding_H
#define traktor_Encoding_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Character encoding.
 * \ingroup Core
 */
class T_DLLCLASS Encoding : public Object
{
	T_RTTI_CLASS(Encoding)

public:
	enum { MaxEncodingSize = 8 };

	/*! \brief Translate from Unicode to special encoding.
	 *
	 * \param chars Unicode source characters.
	 * \param count Number of source characters.
	 * \param out Encoded characters, should be at least count * MaxEncodingSize bytes to ensure enough space.
	 * \return Number of bytes used in out array, -1 if encoding failed.
	 */
	virtual int translate(const wchar_t* chars, int count, uint8_t* out) const = 0;

	/*! \brief Translate from special encoding to Unicode.
	 *
	 * \param in Input character in special encoding.
	 * \param count Number of valid bytes in input buffer.
	 * \param out Output unicode character.
	 * \return -1 if decoding failed, otherwise number of bytes "consumed" from input buffer.
	 */
	virtual int translate(const uint8_t in[MaxEncodingSize], int count, wchar_t& out) const = 0;
};

}

#endif	// traktor_Encoding_H
