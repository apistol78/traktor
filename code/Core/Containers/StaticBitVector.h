/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{

template < int Size >
class StaticBitVector
{
public:
	StaticBitVector(bool initial = false)
	{
		for (uint32_t i = 0; i < (Size + 31) / 32; ++i)
			m_data[i] = initial ? ~0U : 0U;
	}

	void assign(bool value)
	{
		for (uint32_t i = 0; i < (Size + 31) / 32; ++i)
			m_data[i] = value ? ~0U : 0U;
	}

	void set()
	{
		for (uint32_t i = 0; i < (Size + 31) / 32; ++i)
			m_data[i] = ~0U;
	}

	void set(uint32_t index)
	{
		m_data[index / 32] |= (1U << (index & 31));
	}

	void set(uint32_t index, bool value)
	{
		if (value)
			m_data[index / 32] |= (1U << (index & 31));
		else
			m_data[index / 32] &= ~(1U << (index & 31));
	}

	void clear()
	{
		for (uint32_t i = 0; i < (Size + 31) / 32; ++i)
			m_data[i] = 0U;
	}

	void clear(uint32_t index)
	{
		m_data[index / 32] &= ~(1U << (index & 31));
	}

	bool anyClear() const
	{
		for (uint32_t i = 0; i < (Size + 31) / 32; ++i)
		{
			if (m_data[i] != ~0U)
				return true;
		}
		return false;
	}

	bool anySet() const
	{
		for (uint32_t i = 0; i < (Size + 31) / 32; ++i)
		{
			if (m_data[i] != 0U)
				return true;
		}
		return false;
	}

	uint32_t size() const
	{
		return Size;
	}

	bool operator [] (uint32_t index) const
	{
		return (m_data[index / 32] & (1U << (index & 31))) != 0;
	}

	static StaticBitVector< Size > and(const StaticBitVector< Size >& a, StaticBitVector< Size >& b)
	{
		StaticBitVector< Size > r;
		for (uint32_t i = 0; i < (Size + 31) / 32; ++i)
			r.m_data[i] = a.m_data[i] & b.m_data[i];
		return r;
	}

private:
	uint32_t m_data[(Size + 31) / 32];
};

}
