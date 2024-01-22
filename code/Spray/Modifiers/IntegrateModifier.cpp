/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/Modifiers/IntegrateModifier.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.IntegrateModifier", IntegrateModifier, Modifier)

IntegrateModifier::IntegrateModifier(float timeScale, bool linear, bool angular)
:	m_timeScale(timeScale)
,	m_linear(linear)
,	m_angular(angular)
{
}

void IntegrateModifier::update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const
{
	const Scalar scaledDeltaTime = deltaTime * m_timeScale;
	if (m_linear && m_angular)
	{
		for (size_t i = first; i < last; ++i)
		{
			points[i].position += points[i].velocity * Scalar(points[i].inverseMass) * scaledDeltaTime;
			points[i].orientation += points[i].angularVelocity * scaledDeltaTime;
		}
	}
	else if (m_linear)
	{
		for (size_t i = first; i < last; ++i)
			points[i].position += points[i].velocity * Scalar(points[i].inverseMass) * scaledDeltaTime;
	}
	else if (m_angular)
	{
		for (size_t i = first; i < last; ++i)
			points[i].orientation += points[i].angularVelocity * scaledDeltaTime;
	}
}

}
