/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cmath>
#include <limits>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Drawing/Palette.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.drawing.Palette", 0, Palette, Object)

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
	for (auto i = m_colors.begin(); i != m_colors.end(); ++i)
	{
		const float diff =
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

void Palette::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Color4f >(L"colors", m_colors);
}

}
