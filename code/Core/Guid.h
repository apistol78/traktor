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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Globally unique identifier.
 * \ingroup Core
 *
 * Globally unique identifier, with support of translating
 * back and forth from a string representation.
 */
class T_DLLCLASS Guid
{
public:
	const static Guid null;

	Guid();

	/*! Initialize guid from string.
	 * String must be properly formated or else
	 * the guid will be invalid.
	 *
	 * \param s Formatted guid.
	 */
	explicit Guid(const std::wstring& s);

	/*! Initialize guid from bytes. */
	explicit Guid(const uint8_t data[16]);

	/*! Create guid from string. */
	bool create(const std::wstring& s);

	/*! Create unique guid.
	 * Create guid which is "guaranteed" to be
	 * globally unique from various parameters such as mac address etc.
	 */
	static Guid create();

	/*! Format guid string. */
	std::wstring format() const;

	/*! Check if guid is valid. */
	bool isValid() const;

	/*! Check if guid is all zeros.
	 *
	 * Note a null guid is still a valid guid, just
	 * not representative of a guid.
	 */
	bool isNull() const;

	/*! Check if guid is valid and not null. */
	bool isNotNull() const;

	/*! Permutate this guid.
	 * \return Current guid before permutation.
	 */
	Guid permutate(uint32_t iterations = 1);

	/*! Generate a permutation of this guid in N iterations. */
	Guid permutation(uint32_t iterations) const;

	/*! Generate a permutation of this guid combined with another guid. */
	Guid permutation(const Guid& seed) const;

	/*! Convert guid into 16 bytes. */
	operator const uint8_t* () const;

	/*! Equal compare. */
	bool operator == (const Guid& r) const;

	/*! Not equal compare. */
	bool operator != (const Guid& r) const;

	/*! Less than compare. */
	bool operator < (const Guid& r) const;

	/*! Greater than compare. */
	bool operator > (const Guid& r) const;

private:
	uint8_t m_data[16];
	bool m_valid;
};

}

