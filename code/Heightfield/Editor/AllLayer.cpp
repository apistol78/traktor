/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/AllLayer.h"

#include "Heightfield/Heightfield.h"

#include <algorithm>
#include <limits>

namespace traktor::hf
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.AllLayer", 0, AllLayer, ProceduralLayer)

void AllLayer::evaluate(const Heightfield* heightfield, int32_t gridZ, float reach, float* outDistances, float* outHeights) const
{
	// Every sample is infinitely far inside.
	const int32_t size = heightfield->getSize();
	std::fill(outDistances, outDistances + size, std::numeric_limits< float >::max());
	std::fill(outHeights, outHeights + size, 0.0f);
}

void AllLayer::serialize(ISerializer& s)
{
	ProceduralLayer::serialize(s);
}

}
