/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Config.h"

namespace traktor
{

/*! Helper class for Four Character Code identifiers, hence FourCC.
 * \ingroup Core
 *
 * A FourCC code is simple for humans to understand and also fast for
 * computers to compare.
 */
class FourCC
{
public:
	/*! Initialize null FourCC.
	 *
	 * Initialize a null FourCC identifier, should
	 * be considered non-valid for identification purpose
	 * anyway.
	 */
	inline FourCC();

	/*! Copy constructor.
	 *
	 * \param fcc Source FourCC identifier.
	 */
	inline FourCC(const FourCC& fcc);

	/*! Initialize the FourCC from a dword number.
	 *
	 * \param fcc Source dword identifier.
	 */
	inline explicit FourCC(uint32_t fcc);

	/*! Initialize the FourCC from a 4-letter string.
	 *
	 * \param str 4-letter string.
	 */
	inline  FourCC(const char* const str);

	/*! Format into string. */
	inline std::wstring format() const;

	/*! Equal operator.
	 *
	 * Compare if two FourCC identifiers are equal.
	 * This is actually a simple 32 bit integer compare.
	 * \param fcc Right hand FourCC identifier.
	 */
	inline bool operator == (const FourCC& fcc) const;

	/*! Non-equal operator.
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

	/*! FourCC encoded into dword. */
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

inline FourCC::FourCC(const char* const str)
{
	m_fcc = (str[3] << 24) | (str[2] << 16) | (str[1] << 8) | (str[0]);
}

std::wstring FourCC::format() const
{
	wchar_t str[] =
	{
		(wchar_t)((m_fcc) & 0xff),
		(wchar_t)((m_fcc >> 8) & 0xff),
		(wchar_t)((m_fcc >> 16) & 0xff),
		(wchar_t)((m_fcc >> 24) & 0xff),
		0
	};
	return std::wstring(str);
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

