/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb2.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Matrix33.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SVG_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::svg
{

/*! SVG gradient description.
 * \ingroup SVG
 */
class T_DLLCLASS Gradient : public Object
{
	T_RTTI_CLASS;

public:
	enum GradientType
	{
		GtLinear,
		GtRadial
	};

	struct Stop
	{
		float offset;
		Color4f color;
	};

	explicit Gradient(GradientType gradientType);

	GradientType getGradientType() const;

	void addStop(float offset, const Color4f& color);

	uint32_t getStopCount() const;

	float getStopOffset(uint32_t index) const;

	Color4f getStopColor(uint32_t index) const;

	void setStops(const AlignedVector< Stop >& stops);

	const AlignedVector< Stop >& getStops() const;

	void setBounds(const Aabb2& bounds);

	const Aabb2& getBounds() const;

	void setTransform(const Matrix33& transform);

	const Matrix33& getTransform() const;

private:
	GradientType m_gradientType;
	AlignedVector< Stop > m_stops;
	Aabb2 m_bounds;
	Matrix33 m_transform;
};

}
