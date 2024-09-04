/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/Modifiers/SizeModifier.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SizeModifier", SizeModifier, Modifier)

SizeModifier::SizeModifier(float adjustRate)
:	m_adjustRate(adjustRate)
{
}

void SizeModifier::writeSequence(Vector4*& inoutSequence) const
{
	*inoutSequence++ = Vector4(
		OperationCode,
		m_adjustRate,
		0.0f,
		0.0f
	);
}

void SizeModifier::update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const
{
	const float deltaSize = m_adjustRate * deltaTime;
	for (size_t i = first; i < last; ++i)
		points[i].size += deltaSize;
}

}
