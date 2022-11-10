/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/Modifiers/SizeModifier.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SizeModifier", SizeModifier, Modifier)

SizeModifier::SizeModifier(float adjustRate)
:	m_adjustRate(adjustRate)
{
}

void SizeModifier::update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const
{
	float deltaSize = m_adjustRate * deltaTime;
	for (size_t i = first; i < last; ++i)
		points[i].size += deltaSize;
}

	}
}
