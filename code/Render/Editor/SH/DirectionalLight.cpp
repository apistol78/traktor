/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Editor/SH/DirectionalLight.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"DirectionalLight", DirectionalLight, SHFunction)

DirectionalLight::DirectionalLight(const Vector4& direction)
:	m_direction(direction)
{
}

Vector4 DirectionalLight::evaluate(const Polar& direction) const
{
	const Vector4 unit = direction.toUnitCartesian();
	return Vector4(1.0f, 1.0f, 1.0f, 0.0f) * clamp(dot3(unit, m_direction), 0.0_simd, 1.0_simd);
}

}
