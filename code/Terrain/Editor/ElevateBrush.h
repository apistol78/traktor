/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Terrain/Editor/IBrush.h"
#include "Terrain/Editor/TerrainEditData.h"

namespace traktor::drawing
{

class Image;

}

namespace traktor::hf
{

class Heightfield;

}

namespace traktor::terrain
{

class ElevateBrush : public IBrush
{
	T_RTTI_CLASS;

public:
	explicit ElevateBrush(TerrainEditData* data);

	virtual uint32_t begin(float x, float y, const State& state) override final;

	virtual void apply(float x, float y) override final;

	virtual void end(float x, float y) override final;

private:
	Ref< TerrainEditData > m_data;
	int32_t m_radius;
	const IFallOff* m_fallOff;
	double m_strength;
	int32_t m_attribute;
	SmallMap< int32_t, double > m_heights; //!< Unquantized heights of samples touched by stroke.
};

}
