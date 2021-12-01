#pragma once

#include "Core/Config.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{

class BitVector
{
public:
	BitVector() = default;

	BitVector(uint32_t size, bool initial)
	:	m_data(new uint32_t [(size + 31) / 32])
	,	m_size(size)
	{
		for (uint32_t i = 0; i < (m_size + 31) / 32; ++i)
			m_data[i] = initial ? ~0U : 0U;
	}

	BitVector(const BitVector& bv)
	{
		m_data.reset(new uint32_t[bv.m_size]);
		for (uint32_t i = 0; i < bv.m_size; ++i)
			m_data[i] = bv.m_data[i];
		m_size = bv.m_size;
	}

#if defined(T_CXX11)
	BitVector(BitVector&& bv)
	{
		m_data.move(bv.m_data);
		m_size = bv.m_size;
	}
#endif

	void assign(uint32_t size, bool initial)
	{
		if (size > m_size)
			m_data.reset(new uint32_t [(size + 31) / 32]);

		for (uint32_t i = 0; i < (size + 31) / 32; ++i)
			m_data[i] = initial ? ~0U : 0U;

		m_size = size;
	}

	void set()
	{
		for (uint32_t i = 0; i < (m_size + 31) / 32; ++i)
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
		for (uint32_t i = 0; i < (m_size + 31) / 32; ++i)
			m_data[i] = 0U;
	}

	void clear(uint32_t index)
	{
		m_data[index / 32] &= ~(1U << (index & 31));
	}

	uint32_t size() const
	{
		return m_size;
	}

	bool operator [] (uint32_t index) const
	{
		return (m_data[index / 32] & (1U << (index & 31))) != 0;
	}

	BitVector& operator = (const BitVector& bv)
	{
		m_data.reset(new uint32_t[bv.m_size]);
		for (uint32_t i = 0; i < bv.m_size; ++i)
			m_data[i] = bv.m_data[i];
		return *this;
	}

#if defined(T_CXX11)
	BitVector& operator = (BitVector&& bv)
	{
		m_data.move(bv.m_data);
		m_size = bv.m_size;
		return *this;
	}
#endif

private:
	AutoArrayPtr< uint32_t > m_data;
	uint32_t m_size = 0;
};

}
