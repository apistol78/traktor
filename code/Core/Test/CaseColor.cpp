/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Test/CaseColor.h"

#include "Core/Math/Color4f.h"

namespace traktor::test
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseColor", 0, CaseColor, Case)

void CaseColor::run()
{
	Color4f clr(1.0f / 255.0f, 2.0f / 255.0f, 3.0f / 255.0f, 4.0f / 255.0f);
	Color4ub cub = clr.toColor4ub();
	Color4f dst = Color4f::fromColor4ub(cub);

	CASE_ASSERT_EQUAL(cub.r, 1);
	CASE_ASSERT_EQUAL(cub.g, 2);
	CASE_ASSERT_EQUAL(cub.b, 3);
	CASE_ASSERT_EQUAL(cub.a, 4);

	CASE_ASSERT_EQUAL(clr.getRed(), dst.getRed());
	CASE_ASSERT_EQUAL(clr.getGreen(), dst.getGreen());
	CASE_ASSERT_EQUAL(clr.getBlue(), dst.getBlue());
	CASE_ASSERT_EQUAL(clr.getAlpha(), dst.getAlpha());
}

}
