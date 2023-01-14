/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Core/Misc/Key.h"
#include "Core/Misc/String.h"

namespace traktor
{

Key::Key(uint32_t k0, uint32_t k1, uint32_t k2, uint32_t k3)
:	m_kv(k0, k1, k2, k3)
{
}

Key Key::parse(const std::wstring& kf)
{
	if (kf.length() != 8 + 8 + 8 + 8 + 3)
		return Key();

	const std::wstring s0 = kf.substr(0, 8);
	const std::wstring s1 = kf.substr(9, 8);
	const std::wstring s2 = kf.substr(18, 8);
	const std::wstring s3 = kf.substr(27, 8);

	const uint32_t k0 = std::wcstoul(s0.c_str(), nullptr, 16);
	const uint32_t k1 = std::wcstoul(s1.c_str(), nullptr, 16);
	const uint32_t k2 = std::wcstoul(s2.c_str(), nullptr, 16);
	const uint32_t k3 = std::wcstoul(s3.c_str(), nullptr, 16);
	return Key(k0, k1, k2, k3);
}

bool Key::valid() const
{
	return !(*this == Key(0, 0, 0, 0));
}

std::wstring Key::format() const
{
	return str(L"%08x_%08x_%08x_%08x", std::get< 0 >(m_kv), std::get< 1 >(m_kv), std::get< 2 >(m_kv), std::get< 3 >(m_kv));
}

Key Key::read(IStream* stream)
{
	uint32_t kv[4];
	if (stream->read(kv, sizeof(kv)) == sizeof(kv))
		return Key(kv[0], kv[1], kv[2], kv[3]);
	else
		return Key();
}

bool Key::write(IStream* stream) const
{
	const uint32_t kv[4] = { std::get< 0 >(m_kv), std::get< 1 >(m_kv), std::get< 2 >(m_kv), std::get< 3 >(m_kv) };
	return stream->write(kv, sizeof(kv)) == sizeof(kv);
}

bool Key::operator == (const Key& rh) const
{
	return m_kv == rh.m_kv;
}

bool Key::operator < (const Key& rh) const
{
	return m_kv < rh.m_kv;
}

bool Key::operator > (const Key& rh) const
{
	return m_kv > rh.m_kv;
}

}
