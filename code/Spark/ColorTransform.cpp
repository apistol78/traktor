/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spark/ColorTransform.h"

namespace traktor::spark
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ColorTransform", ColorTransform, Object)

ColorTransform::ColorTransform()
:	mul(Color4f(1.0f, 1.0f, 1.0f, 1.0f))
,	add(Color4f(0.0f, 0.0f, 0.0f, 0.0f))
{
}

ColorTransform::ColorTransform(const ColorTransform& cxform)
:	mul(cxform.mul)
,	add(cxform.add)
{
}

ColorTransform::ColorTransform(const Color4f& mul_)
:	mul(mul_)
,	add(Color4f(0.0f, 0.0f, 0.0f, 0.0f))
{
}

ColorTransform::ColorTransform(const Color4f& mul_, const Color4f& add_)
:	mul(mul_)
,	add(add_)
{
}

void ColorTransform::serialize(ISerializer& s)
{
	s >> Member< Color4f >(L"mul", mul);
	s >> Member< Color4f >(L"add", add);
}

}
