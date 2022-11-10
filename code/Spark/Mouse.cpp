/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Class/CastAny.h"
#include "Spark/Mouse.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Mouse", Mouse, Object)

void Mouse::eventMouseDown(int32_t x, int32_t y, int32_t button)
{
	m_eventButtonDown.issue();
}

void Mouse::eventMouseUp(int32_t x, int32_t y, int32_t button)
{
	m_eventButtonUp.issue();
}

void Mouse::eventMouseMove(int32_t x, int32_t y, int32_t button)
{
	m_eventMouseMove.issue();
}

void Mouse::eventMouseWheel(int32_t x, int32_t y, int32_t delta)
{
	Any argv = CastAny< int >::set(delta);
	m_eventMouseWheel.issue(1, &argv);
}

	}
}
