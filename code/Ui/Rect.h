/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Point.h"
#include "Ui/Size.h"

namespace traktor::ui
{

/*! Rectangle
 * \ingroup UI
 */
class Rect
{
public:
	int32_t left;
	int32_t top;
	int32_t right;
	int32_t bottom;

	inline Rect();

	inline Rect(int32_t l, int32_t t, int32_t r, int32_t b);

	inline Rect(const Point& tl, const Point& br);

	inline Rect(const Point& tl, const Size& wh);

	inline Rect(const Rect& rc);

	inline void moveTo(const Point& pnt);

	inline void setWidth(int32_t width);

	inline int32_t getWidth() const;

	inline void setHeight(int32_t height);

	inline int32_t getHeight() const;

	inline void setSize(const Size& size);

	inline Size getSize() const;

	inline Point getTopLeft() const;

	inline Point getTopRight() const;

	inline Point getBottomLeft() const;

	inline Point getBottomRight() const;

	inline Point getCenter() const;

	inline Rect getUnified() const;

	inline Rect offset(const Size& sz) const;

	inline Rect offset(int32_t x, int32_t y) const;

	inline Rect inflate(const Size& sz) const;

	inline Rect inflate(int32_t x, int32_t y) const;

	inline int32_t area() const;

	inline Rect contain(const Point& pnt) const;

	inline bool inside(const Point& pnt, bool inclusive = true) const;

	inline bool intersect(const Rect& rc) const;

	inline Rect dpi96() const;

	inline Rect invdpi96() const;

	inline Rect& operator = (const Rect& rc);

	inline bool operator == (const Rect& rc) const;

	inline bool operator != (const Rect& rc) const;
};

}

#include "Ui/Rect.inl"

