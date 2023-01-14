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
#include <tuple>
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

class IStream;

/*! 128-bit key with formatting, parsing and serialization.
 * \ingroup Core
 */
class T_DLLCLASS Key
{
public:
	Key() = default;

	Key(uint32_t k0, uint32_t k1, uint32_t k2, uint32_t k3);

	static Key parse(const std::wstring& kf);

	bool valid() const;

	std::wstring format() const;

	static Key read(IStream* stream);

	bool write(IStream* stream) const;

	bool operator == (const Key& rh) const;

	bool operator < (const Key& rh) const;

	bool operator > (const Key& rh) const;

private:
	std::tuple< uint32_t, uint32_t, uint32_t, uint32_t > m_kv = { 0, 0, 0, 0 };
};

}
