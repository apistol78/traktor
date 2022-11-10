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
#include "Core/Math/Color4f.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializer;

	namespace spark
	{

/*! Color transform wrapper.
 * \ingroup Spark
 */
class T_DLLCLASS ColorTransform : public Object
{
	T_RTTI_CLASS;

public:
	Color4f mul;
	Color4f add;

	ColorTransform();

	ColorTransform(const ColorTransform& cxform);

	explicit ColorTransform(const Color4f& mul);

	explicit ColorTransform(const Color4f& mul, const Color4f& add);

	void serialize(ISerializer& s);

	ColorTransform operator * (const ColorTransform& rh) const
	{
		ColorTransform cxtr;
		cxtr.mul = mul * rh.mul;
		cxtr.add = (mul * rh.add + add).saturated();
		return cxtr;
	}

	ColorTransform& operator = (const ColorTransform& rh)
	{
		mul = rh.mul;
		add = rh.add;
		return *this;
	}
};

	}
}

