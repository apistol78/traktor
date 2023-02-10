/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Math/Color4f.h"

namespace traktor
{
	namespace terrain
	{

class IFallOff;

class IBrush : public Object
{
	T_RTTI_CLASS;

public:
	enum Mode
	{
		MdMaterial = 1,
		MdColor = 2,
		MdHeight = 4,
		MdCut = 8,
		MdAttribute = 16
	};

	struct State
	{
		int32_t radius = 0;
		const IFallOff* falloff = nullptr;
		float strength = 0.0f;
		Color4f color;
		int32_t material = 0;
		int32_t attribute = 0;
	};

	virtual uint32_t begin(float x, float y, const State& state) = 0;

	virtual void apply(float x, float y) = 0;

	virtual void end(float x, float y) = 0;
};

	}
}

