/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cmath>
#include <limits>
#include "Drawing/Palette.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.Palette", Palette, Object)

Palette::Palette(int32_t size)
:	m_colors(size)
{
}

int32_t Palette::getSize() const
{
	return int32_t(m_colors.size());
}

void Palette::set(int32_t index, const Color4f& c)
{
	m_colors[index] = c;
}

const Color4f& Palette::get(int32_t index) const
{
	return m_colors[index];
}

int32_t Palette::find(const Color4f& c, bool exact) const
{
	std::pair< float, int32_t > mn(std::numeric_limits< float >::max(), -1);
	for (AlignedVector< Color4f >::const_iterator i = m_colors.begin(); i != m_colors.end(); ++i)
	{
		float diff =
			std::abs(c.getRed() - i->getRed()) +
			std::abs(c.getGreen() - i->getGreen()) +
			std::abs(c.getBlue() - i->getBlue()) +
			std::abs(c.getAlpha() - i->getAlpha());
			
		if (diff < mn.first)
		{
			mn.first = diff;
			mn.second = int32_t(std::distance(m_colors.begin(), i));
		}
	}
	return (exact && mn.first != 0.0f) ? -1 : mn.second;
}
	
	}
}
