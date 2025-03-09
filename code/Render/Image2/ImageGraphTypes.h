/*
 * TRAKTOR
 * Copyright (c) 2023-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/StaticVector.h"
#include "Render/Frame/RenderGraphTypes.h"
#include "Render/Types.h"

namespace traktor::render
{

typedef handle_t img_handle_t;
typedef StaticVector< RGTargetSet, 32 > targetSetVector_t;
typedef StaticVector< handle_t, 32 > bufferVector_t;

enum class BlurType
{
	// 1D separable
	Gaussian,
	Sine,
	Box,
	// 2D combined
	Box2D,
	Circle2D
};

enum class WorkSize
{
	Manual, //!< Manual size.
	Output, //!< Size of output.
	SizeOf	//!< Size of reference input.
};

struct PassOutput
{
	int32_t targetSet = -1;
	int32_t sbuffer = -1;

	PassOutput() = default;

	PassOutput(int32_t targetSet_, int32_t sbuffer_)
		: targetSet(targetSet_)
		, sbuffer(sbuffer_)
	{
		T_ASSERT(!(targetSet >= 0 && sbuffer >= 0));
	}
};

}
