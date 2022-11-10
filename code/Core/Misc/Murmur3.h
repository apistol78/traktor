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

/* Murmur3 (32) checksum.
 * \ingroup Core
 */
class T_DLLCLASS Murmur3 : public IHash
{
	T_RTTI_CLASS;

public:
	Murmur3(uint32_t seed = 0);

	virtual void begin() override final;

	virtual void feed(const void* buffer, uint64_t bufferSize) override final;

	virtual void end() override final;

	uint32_t get() const;

	template < typename T >
	void feed(const T& value)
	{
		feed(&value, sizeof(value));
	}

	void feed(const std::wstring& value)
	{
		if (!value.empty())
			feed(value.c_str(), (uint64_t)(value.length() * sizeof(wchar_t)));
	}

private:
	uint8_t m_data[4];
	uint8_t m_ndata;
	uint32_t m_seed;
	uint32_t m_tlen;
	uint32_t m_h;
};

}

