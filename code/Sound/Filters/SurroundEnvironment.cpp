/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Sound/Filters/SurroundEnvironment.h"

namespace traktor::sound
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SurroundEnvironment", SurroundEnvironment, Object)

SurroundEnvironment::SurroundEnvironment(
	float maxDistance,
	float innerRadius,
	float fallOffExponent,
	float verticalScale,
	bool fullSurround
)
:	m_maxDistance(maxDistance)
,	m_innerRadius(innerRadius)
,	m_fallOffExponent(fallOffExponent)
,	m_verticalAxisScale(1.0f, verticalScale, 1.0f, 0.0f)
,	m_fullSurround(fullSurround)
{
}

void SurroundEnvironment::setMaxDistance(float maxDistance)
{
	m_maxDistance = Scalar(maxDistance);
}

void SurroundEnvironment::setInnerRadius(float innerRadius)
{
	m_innerRadius = Scalar(innerRadius);
}

void SurroundEnvironment::setFallOffExponent(float fallOffExponent)
{
	m_fallOffExponent = Scalar(fallOffExponent);
}

void SurroundEnvironment::setVerticalScale(float verticalScale)
{
	m_verticalAxisScale = Vector4(1.0f, verticalScale, 1.0f, 0.0f);
}

void SurroundEnvironment::setFullSurround(bool fullSurround)
{
	m_fullSurround = fullSurround;
}

void SurroundEnvironment::setListenerTransforms(const listenerTransformVector_t& listenerTransforms)
{
	m_listenerTransforms = listenerTransforms;
}

}
