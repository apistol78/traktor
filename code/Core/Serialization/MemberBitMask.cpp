/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberBitMask.h"

namespace traktor
{

MemberBitMask::MemberBitMask(const wchar_t* const name, uint32_t& bm, const Bit* bits)
:	MemberComplex(name, true)
,	m_bits(bits)
,	m_bm(bm)
{
}

MemberBitMask::MemberBitMask(const wchar_t* const name, uint32_t& bm, const Bit* bits, const Attribute& attributes)
:	MemberComplex(name, true, attributes)
,	m_bits(bits)
,	m_bm(bm)
{
}

void MemberBitMask::serialize(ISerializer& s) const
{
	if (s.getDirection() == ISerializer::Direction::Read)
		m_bm = 0;
	for (int i = 0; m_bits[i].id; ++i)
	{
		bool bv = bool((m_bm & m_bits[i].mask) != 0);
		s >> Member< bool >(m_bits[i].id, bv);
		m_bm |= bv ? m_bits[i].mask : 0;
	}
}

}
