/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/Modifiers/BrownianModifier.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.BrownianModifier", BrownianModifier, Modifier)

BrownianModifier::BrownianModifier(float factor)
:	m_factor(factor)
{
}

void BrownianModifier::update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const
{
	for (size_t i = first; i < last; ++i)
	{
		Vector4 r(
			m_random.nextFloat() * 2.0f - 1.0f,
			m_random.nextFloat() * 2.0f - 1.0f,
			m_random.nextFloat() * 2.0f - 1.0f
		);
		points[i].velocity += (r * m_factor * deltaTime) * Scalar(points[i].inverseMass);
	}
}

	}
}
