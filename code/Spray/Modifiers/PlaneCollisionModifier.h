/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Plane.h"
#include "Spray/Modifier.h"

namespace traktor
{
	namespace spray
	{

/*! Plane collision modifier.
 * \ingroup Spray
 */
class PlaneCollisionModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	explicit PlaneCollisionModifier(const Plane& plane, float radius, float restitution);

	virtual void update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const override final;

private:
	Plane m_plane;
	Scalar m_radius;
	Scalar m_restitution;
};

	}
}

