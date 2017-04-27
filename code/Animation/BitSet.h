/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
				int start = 0;
				if (!(m_bits[i] & 0x000000ff))
				{
					start = 8;
					if (!(m_bits[i] & 0x0000ffff))
					{
						start = 16;
						if (!(m_bits[i] & 0x00ffffff))
							start = 24;
					}
				}

				int end = 32;
				if (!(m_bits[i] & 0xff000000))
				{
					end = 24;
					if (!(m_bits[i] & 0xffff0000))
					{
						end = 16;
						if (!(m_bits[i] & 0xffffff00))
							end = 8;
					}
				}

				outMin += start;
				for (int j = start; j < end; ++j)
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
				int end = 0;
				if (!(m_bits[i] & 0x000000ff))
				{
					end = 8;
					if (!(m_bits[i] & 0x0000ffff))
					{
						end = 16;
						if (!(m_bits[i] & 0x00ffffff))
							end = 24;
					}
				}

				int start = 31;
				if (!(m_bits[i] & 0xff000000))
				{
					start = 23;
					if (!(m_bits[i] & 0xffff0000))
					{
						start = 15;
						if (!(m_bits[i] & 0xffffff00))
							start = 7;
					}
				}

				outMax -= (31 - start);
				for (int j = start; j >= end; --j)
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
