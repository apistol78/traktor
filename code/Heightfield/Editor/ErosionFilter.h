/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

namespace traktor::hf
{

class Heightfield;

/*! Erosion filter.
 * \ingroup Heightfield
 *
 * Water droplets, flowing downhill, erode and deposit sediment.
 * http://ranmantaru.com/blog/2011/10/08/water-erosion-on-heightmap-terrain/
 */
class ErosionFilter : public Object
{
	T_RTTI_CLASS;

public:
	struct Parameters
	{
		int32_t iterations = 100;	//!< Number of water droplets, times 1k.
		float capacity = 10.0f;		//!< Sediment carried per drop, speed and water of droplet.
		float minSlope = 0.05f;		//!< Least drop, in metres per move, used for capacity; droplets on flats still carry sediment.
		float evaporation = 0.001f; //!< Fraction of water evaporated each move.
		float erosion = 0.9f;		//!< Fraction of free capacity eroded each move.
		float deposition = 0.02f;	//!< Fraction of sediment beyond capacity deposited each move.
		float inertia = 0.1f;		//!< Fraction of direction kept each move; higher make smoother turns of channels.
		float gravity = 40.0f;		//!< Acceleration of droplets by drop.
	};

	explicit ErosionFilter(const Parameters& parameters);

	void apply(Heightfield* heightfield) const;

private:
	Parameters m_parameters;
};

}
