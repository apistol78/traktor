/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Misc/Murmur3.h"

namespace traktor
{
	namespace
	{

static inline uint32_t murmur_32_scramble(uint32_t k)
{
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Murmur3", Murmur3, IHash)

Murmur3::Murmur3(uint32_t seed)
:   m_ndata(0)
,   m_seed(seed)
,   m_tlen(0)
,	m_h(0)
{
}

void Murmur3::begin()
{
    m_ndata = 0;
    m_tlen = 0;
	m_h = m_seed;
}

void Murmur3::feed(const void* buffer, uint64_t bufferSize)
{
	const uint8_t* start = (const uint8_t*)buffer;
    const uint8_t* key = start;
    uint32_t k;

    if (!buffer)
        return;

    while (key < &start[bufferSize])
    {
        while (m_ndata < 4 && key < &start[bufferSize])
        {
            m_data[m_ndata] = *key++;
            m_ndata++;
        }
        if (m_ndata >= 4)
        {
            // Here is a source of differing results across endiannesses.
            // A swap here has no effects on hash properties though.
            std::memcpy(&k, m_data, sizeof(uint32_t));
            m_h ^= murmur_32_scramble(k);
            m_h = (m_h << 13) | (m_h >> 19);
            m_h = m_h * 5 + 0xe6546b64;
            m_ndata = 0;
        }
    }

    m_tlen += (uint32_t)bufferSize;
}

void Murmur3::end()
{
    // Read the rest.
    uint32_t k = 0;
    for ( ; m_ndata; --m_ndata)
    {
        k <<= 8;
        k |= m_data[m_ndata - 1];
    }

    // A swap is *not* necessary here because the preceding loop already
    // places the low bytes in the low places according to whatever endianness
    // we use. Swaps only apply when the memory is copied in a chunk.
    m_h ^= murmur_32_scramble(k);

    // Finalize.
	m_h ^= m_tlen;
	m_h ^= m_h >> 16;
	m_h *= 0x85ebca6b;
	m_h ^= m_h >> 13;
	m_h *= 0xc2b2ae35;
	m_h ^= m_h >> 16;
}

uint32_t Murmur3::get() const
{
	return m_h;
}

}
