/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Spray/Modifier.h"

namespace traktor
{
	namespace spray
	{

/*! Gravity modifier.
 * \ingroup Spray
 */
class GravityModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	explicit GravityModifier(const Vector4& gravity, bool world);

	virtual void update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const override final;

private:
	Vector4 m_gravity;
	bool m_world;
};

	}
}

