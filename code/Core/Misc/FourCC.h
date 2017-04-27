/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef _traktor_FourCC_H
#define _traktor_FourCC_H

#include "Core/Config.h"

namespace traktor
{

/*! \brief Helper class for Four Character Code identifiers, hence FourCC.
 * \ingroup Core
 *
 * A FourCC code is simple for humans to understand and also fast for
 * computers to compare.
 */
class FourCC
{
public:
	/*! \brief Initialize null FourCC.
	 *
	 * Initialize a null FourCC identifier, should
	 * be considered non-valid for identification purpose
	 * anyway.
	 */
	inline FourCC();

	/*! \brief Copy constructor.
	 *
	 * \param fcc Source FourCC identifier.
	 */
	inline FourCC(const FourCC& fcc);
	
	/*! \brief Initialize the FourCC from a dword number.
	 *
	 * \param fcc Source dword identifier.
	 */
	inline explicit FourCC(uint32_t fcc);
	
	/*! \brief Initialize the FourCC from a 4-letter string.
	 *
	 * \param str 4-letter string.
	 */
	inline explicit FourCC(const char* str);

	/*! \brief Format into string. */
	inline std::string format() const;
	
	/*! \brief Equal operator.
	 *
	 * Compare if two FourCC identifiers are equal.
	 * This is actually a simple 32 bit integer compare.
	 * \param fcc Right hand FourCC identifier.
	 */
	inline bool operator == (const FourCC& fcc) const;

	/*! \brief Non-equal operator.
	 *
	 * Compare if two FourCC identifiers are not equal.
	 * This is actually a simple 32 bit integer compare.
	 * \param fcc Right hand FourCC identifier.
	 */
	inline bool operator != (const FourCC& fcc) const;

	/*!
	 */
	inline bool operator < (const FourCC& fcc) const;

	/*!
	 */
	inline bool operator > (const FourCC& fcc) const;

	/*! \brief FourCC encoded into dword. */
	inline operator uint32_t () const;

private:
	uint32_t m_fcc;
};

inline FourCC::FourCC()
:	m_fcc(0)
{
}

inline FourCC::FourCC(const FourCC& fcc)
:	m_fcc(fcc.m_fcc)
{
}

inline FourCC::FourCC(uint32_t fcc)
:	m_fcc(fcc)
{
}

inline FourCC::FourCC(const char* str)
{
	m_fcc = (str[0] << 24) | (str[1] << 16) | (str[2] << 8) | (str[3]);
}

std::string FourCC::format() const
{
	char str[] =
	{
		(m_fcc >> 24) & 0xff,
		(m_fcc >> 16) & 0xff,
		(m_fcc >> 8) & 0xff,
		(m_fcc) & 0xff,
		0
	};
	return std::string(str);
}

inline bool FourCC::operator == (const FourCC& fcc) const
{
	return (bool)(m_fcc == fcc.m_fcc);
}

inline bool FourCC::operator != (const FourCC& fcc) const
{
	return (bool)(m_fcc != fcc.m_fcc);
}

inline bool FourCC::operator < (const FourCC& fcc) const
{
	return (bool)(m_fcc < fcc.m_fcc);
}

inline bool FourCC::operator > (const FourCC& fcc) const
{
	return (bool)(m_fcc > fcc.m_fcc);
}

inline FourCC::operator uint32_t () const
{
	return m_fcc;
}

}

#endif	// _traktor_FourCC_H
