/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/Modifiers/DragModifier.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.DragModifier", DragModifier, Modifier)

DragModifier::DragModifier(float linearDrag, float angularDrag)
:	m_linearDrag(linearDrag)
,	m_angularDrag(angularDrag)
{
}

void DragModifier::update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const
{
	Scalar dv = 1.0_simd - m_linearDrag * deltaTime;
	float da = 1.0f - m_angularDrag * deltaTime;

	for (size_t i = first; i < last; ++i)
	{
		points[i].velocity *= dv;
		points[i].angularVelocity *= da;
	}
}

	}
}
