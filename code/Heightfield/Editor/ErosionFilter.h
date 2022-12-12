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

namespace traktor::hf
{

class Heightfield;

/*! Erosion filter.
 * \ingroup Heightfield
 */
class ErosionFilter : public Object
{
	T_RTTI_CLASS;

public:
	explicit ErosionFilter(int32_t iterations);

	void apply(Heightfield* heightfield) const;

private:
	int32_t m_iterations;
};

}
