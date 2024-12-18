/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Interval transform.
 * \ingroup Core
 *
 * Interval transform which makes it easier to
 * maintain timing independent animations from
 * discrete transformation updates.
 */
class T_MATH_ALIGN16 T_DLLCLASS IntervalTransform
{
public:
	T_MATH_INLINE IntervalTransform() = default;

	T_MATH_INLINE IntervalTransform(const Transform& transform);

	T_MATH_INLINE IntervalTransform(const Transform& transform0, const Transform& transform1);

	T_MATH_INLINE void set(const Transform& transform);

	T_MATH_INLINE Transform get(float interval) const;

	T_MATH_INLINE void step();

	T_MATH_INLINE const Transform& get0() const;

	T_MATH_INLINE const Transform& get() const;

private:
	Transform m_transform[2];
	bool m_stepped = false;
};

}

#if defined(T_MATH_USE_INLINE)
#	include "Core/Math/Std/IntervalTransform.inl"
#endif

