/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Weather/Clouds/CloudMask.h"

namespace traktor::weather
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.CloudMask", CloudMask, Object)

CloudMask::CloudMask(int32_t size)
:	m_size(size)
{
	m_data.reset(new Sample [size * size]);
}

int32_t CloudMask::getSize() const
{
	return m_size;
}

CloudMask::Sample CloudMask::getSample(int32_t x, int32_t y) const
{
	if (x >= 0 && x < m_size && y >= 0 && y <= m_size)
		return m_data[x + y * m_size];
	else
		return Sample();
}

}
