#ifndef traktor_animation_BitSet_H
#define traktor_animation_BitSet_H

#include "Core/Config.h"

namespace traktor
{
	namespace animation
	{

/*! \brief Bit set class.
 * \ingroup Animation
 *
 * Helper class for maintaining a bit set.
 */
class BitSet
{
public:
	T_FORCE_INLINE BitSet()
	{
		for (int i = 0; i < sizeof_array(m_bits); ++i)
			m_bits[i] = 0x00000000;
	}
	
	T_FORCE_INLINE void set(uint8_t index)
	{
		m_bits[index >> 5] |= (1 << (index & 31));
	}
	
	T_FORCE_INLINE void insert(const BitSet& set)
	{
		for (int i = 0; i < sizeof_array(m_bits); ++i)
			m_bits[i] |= set.m_bits[i];
	}
	
	T_FORCE_INLINE void range(int& outMin, int& outMax) const
	{
		outMin = 0;
		outMax = sizeof_array(m_bits) * 32;

		for (int i = 0; i < sizeof_array(m_bits); ++i)
		{
			if (m_bits[i])
			{
				for (int j = 0; j < 32; ++j)
				{
					if (m_bits[i] & (1 << j))
						break;
					++outMin;
				}
				break;
			}
			outMin += 32;
		}

		for (int i = sizeof_array(m_bits) - 1; i >= 0; --i)
		{
			if (m_bits[i])
			{
				for (int j = 31; j >= 0; --j)
				{
					if (m_bits[i] & (1 << j))
						break;
					--outMax;
				}
				break;
			}
			outMax -= 32;
		}
	}
	
	T_FORCE_INLINE bool operator () (int index) const
	{
		return (m_bits[index >> 5] & (1 << (index & 31))) != 0;
	}
	
private:
	uint32_t m_bits[8];
};

	}
}

#endif	// traktor_animation_BitSet_H
