/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Drawing/Image.h"
#include "Drawing/Filters/ChainFilter.h"

namespace traktor::drawing
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.ChainFilter", ChainFilter, IImageFilter)

void ChainFilter::add(IImageFilter* filter)
{
	m_filters.push_back(filter);
}

void ChainFilter::apply(Image* image) const
{
	for (auto filter : m_filters)
		image->apply(filter);
}

}
