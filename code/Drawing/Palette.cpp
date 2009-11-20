#include <cmath>
#include <limits>
#include "Drawing/Palette.h"

namespace traktor
{
	namespace drawing
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.Palette", Palette, Object)

Palette::Palette(int size)
:	m_colors(size)
{
}

int Palette::getSize() const
{
	return int(m_colors.size());
}

void Palette::set(int index, const Color& c)
{
	m_colors[index] = c;
}

const Color& Palette::get(int index) const
{
	return m_colors[index];
}

int Palette::find(const Color& c, bool exact) const
{
	std::pair< float, int > mn(std::numeric_limits< float >::max(), -1);
	for (std::vector< Color >::const_iterator i = m_colors.begin(); i != m_colors.end(); ++i)
	{
		float diff =
			std::abs(c.getRed() - i->getRed()) +
			std::abs(c.getGreen() - i->getGreen()) +
			std::abs(c.getBlue() - i->getBlue()) +
			std::abs(c.getAlpha() - i->getAlpha());
			
		if (diff < mn.first)
		{
			mn.first = diff;
			mn.second = int(std::distance(m_colors.begin(), i));
		}
	}
	return (exact && mn.first != 0.0f) ? -1 : mn.second;
}
	
	}
}
