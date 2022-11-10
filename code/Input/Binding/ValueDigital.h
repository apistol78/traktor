/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/MathUtils.h"

namespace traktor
{
	namespace input
	{

/*! \brief
 * \ingroup Input
 */
bool T_FORCE_INLINE asBoolean(float inputValue)
{
	return inputValue >= 0.5f ? true : false;
}

/*! \brief
 * \ingroup Input
 */
float T_FORCE_INLINE asFloat(bool inputValue)
{
	return inputValue ? 1.0f : 0.0f;
}

	}
}

