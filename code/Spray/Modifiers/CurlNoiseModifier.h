/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Random.h"
#include "Spray/Modifier.h"

namespace traktor::spray
{

/*! Curl noise modifier.
 * \ingroup Spray
 */
class CurlNoiseModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	explicit CurlNoiseModifier(float factor);

	virtual void update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const override final;

private:
	Scalar m_factor;
	mutable Random m_random;
};

}
