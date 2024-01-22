/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Spray/Modifier.h"

namespace traktor::spray
{

/*! Vortex modifier.
 * \ingroup Spray
 */
class VortexModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	explicit VortexModifier(
		const Vector4& axis,
		float tangentForce,
		float normalConstantForce,
		float normalDistance,
		float normalDistanceForce,
		bool world
	);

	virtual void update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const override final;

private:
	Vector4 m_axis;
	Scalar m_tangentForce;			//< Amount of force applied to each particle in tangent direction.
	Scalar m_normalConstantForce;	//< Amount of constant force applied to each particle in normal direction.
	Scalar m_normalDistance;		//< Distance from axis for each particle, scaled together with m_normalDistanceForce in order to apply different amount of force based on distance.
	Scalar m_normalDistanceForce;
	bool m_world;
};

}
