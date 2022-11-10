/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Misc/IHash.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! SHA-1 hash.
 * \ingroup Core
 */
class T_DLLCLASS SHA1 : public IHash
{
	T_RTTI_CLASS;

public:
	SHA1();

	virtual ~SHA1();

	bool createFromString(const std::wstring& str);

	/*! Begin feeding data for SHA1 checksum calculation. */
	virtual void begin() override final;

	/*! Feed data to SHA1 checksum calculation.
	 *
	 * \param buffer Pointer to data.
	 * \param bufferSize Amount of data in bytes.
	 */
	virtual void feed(const void* buffer, uint64_t bufferSize) override final;

	/*! End feeding data for SHA1 checksum calculation. */
	virtual void end() override final;

	/*! Format MD5 checksum as string. */
	std::wstring format() const;

private:
	void* m_sha1nfo;
};

}

