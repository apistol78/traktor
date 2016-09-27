#ifndef traktor_BitVector_H
#define traktor_BitVector_H

#include "Core/Config.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{

class BitVector
{
public:
	inline BitVector()
	:	m_size(0)
	{
	}

	inline BitVector(uint32_t size, bool initial)
	:	m_data(new uint32_t [(size + 31) / 32])
	,	m_size(size)
	{
		for (uint32_t i = 0; i < (m_size + 31) / 32; ++i)
			m_data[i] = initial ? ~0U : 0U;
	}

	inline BitVector(const BitVector& bv)
	{
		m_data.reset(new uint32_t[bv.m_size]);
		for (uint32_t i = 0; i < bv.m_size; ++i)
			m_data[i] = bv.m_data[i];
	}

#if defined(T_CXX11)
	inline BitVector(BitVector&& bv)
	{
		m_data.move(bv.m_data);
		m_size = bv.m_size;
	}
#endif

	inline void assign(uint32_t size, bool initial)
	{
		if (size > m_size)
			m_data.reset(new uint32_t [(size + 31) / 32]);

		for (uint32_t i = 0; i < (size + 31) / 32; ++i)
			m_data[i] = initial ? ~0U : 0U;

		m_size = size;
	}

	inline void set()
	{
		for (uint32_t i = 0; i < (m_size + 31) / 32; ++i)
			m_data[i] = ~0U;
	}

	inline void set(uint32_t index)
	{
		m_data[index / 32] |= (1U << (index & 31));
	}

	inline void set(uint32_t index, bool value)
	{
		if (value)
			m_data[index / 32] |= (1U << (index & 31));
		else
			m_data[index / 32] &= ~(1U << (index & 31));
	}

	inline void clear()
	{
		for (uint32_t i = 0; i < (m_size + 31) / 32; ++i)
			m_data[i] = 0U;
	}

	inline void clear(uint32_t index)
	{
		m_data[index / 32] &= ~(1U << (index & 31));
	}

	inline uint32_t size() const
	{
		return m_size;
	}

	inline bool operator [] (uint32_t index) const
	{
		return (m_data[index / 32] & (1U << (index & 31))) != 0;
	}

	inline BitVector& operator = (const BitVector& bv)
	{
		m_data.reset(new uint32_t[bv.m_size]);
		for (uint32_t i = 0; i < bv.m_size; ++i)
			m_data[i] = bv.m_data[i];
		return *this;
	}

#if defined(T_CXX11)
	inline BitVector& operator = (BitVector&& bv)
	{
		m_data.move(bv.m_data);
		m_size = bv.m_size;
		return *this;
	}
#endif

private:
	AutoArrayPtr< uint32_t > m_data;
	uint32_t m_size;
};

}

#endif	// traktor_BitVector_H
