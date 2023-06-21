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

 // import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Rectangle
 * \ingroup UI
 */
class T_DLLCLASS Rect
{
public:
	int32_t left = 0;
	int32_t top = 0;
	int32_t right = 0;
	int32_t bottom = 0;

	Rect() = default;

	Rect(int32_t l, int32_t t, int32_t r, int32_t b)
	:	left(l)
	,	top(t)
	,	right(r)
	,	bottom(b)
	{
	}

	Rect(const Point& tl, const Point& br)
	:	left(tl.x)
	,	top(tl.y)
	,	right(br.x)
	,	bottom(br.y)
	{
	}

	Rect(const Point& tl, const Size& wh)
	:	left(tl.x)
	,	top(tl.y)
	,	right(tl.x + wh.cx)
	,	bottom(tl.y + wh.cy)
	{
	}

	Rect(const Rect& rc)
	:	left(rc.left)
	,	top(rc.top)
	,	right(rc.right)
	,	bottom(rc.bottom)
	{
	}

	void moveTo(const Point& pnt)
	{
		const Size size = getSize();
		left = pnt.x;
		top = pnt.y;
		right = pnt.x + size.cx;
		bottom = pnt.y + size.cy;
	}

	void setWidth(int32_t width)
	{
		right = left + width;
	}

	int32_t getWidth() const
	{
		return right - left;
	}

	void setHeight(int32_t height)
	{
		bottom = top + height;
	}

	int32_t getHeight() const
	{
		return bottom - top;
	}

	void setSize(const Size& size)
	{
		setWidth(size.cx);
		setHeight(size.cy);
	}

	Size getSize() const
	{
		return Size(getWidth(), getHeight());
	}

	Point getTopLeft() const
	{
		return Point(left, top);
	}

	Point getTopRight() const
	{
		return Point(right, top);
	}

	Point getBottomLeft() const
	{
		return Point(left, bottom);
	}

	Point getBottomRight() const
	{
		return Point(right, bottom);
	}

	Point getCenter() const
	{
		return Point((left + right) / 2, (top + bottom) / 2);
	}

	Rect getUnified() const
	{
		Rect rc = *this;
		if (rc.left > rc.right)
			std::swap(rc.left, rc.right);
		if (rc.top > rc.bottom)
			std::swap(rc.top, rc.bottom);
		return rc;
	}

	Rect offset(const Size& sz) const
	{
		return offset(sz.cx, sz.cy);
	}

	Rect offset(int32_t x, int32_t y) const
	{
		return Rect(left + x, top + y, right + x, bottom + y);
	}

	Rect inflate(const Size& sz) const
	{
		return inflate(sz.cx, sz.cy);
	}

	Rect inflate(int32_t x, int32_t y) const
	{
		return Rect(left - x, top - y, right + x, bottom + y);
	}

	int32_t area() const
	{
		return getWidth() * getHeight();
	}

	Rect contain(const Point& pnt) const
	{
		Rect rc = *this;
		if (pnt.x < rc.left)
			rc.left = pnt.x;
		if (pnt.y < rc.top)
			rc.top = pnt.y;
		if (pnt.x > rc.right)
			rc.right = pnt.x;
		if (pnt.y > rc.bottom)
			rc.bottom = pnt.y;
		return rc;
	}

	bool inside(const Point& pnt, bool inclusive = true) const
	{
		return
			inclusive ?
			bool(pnt.x >= left && pnt.x <= right && pnt.y >= top && pnt.y <= bottom) :
			bool(pnt.x > left && pnt.x < right && pnt.y > top && pnt.y < bottom);
	}

	bool intersect(const Rect& rc) const
	{
		return (left < rc.right) && (top < rc.bottom) && (right > rc.left) && (bottom > rc.top);
	}

	Rect& operator = (const Rect& rc)
	{
		left = rc.left;
		top = rc.top;
		right = rc.right;
		bottom = rc.bottom;
		return *this;
	}

	bool operator == (const Rect& rc) const
	{
		return
			left == rc.left &&
			top == rc.top &&
			right == rc.right &&
			bottom == rc.bottom;
	}

	bool operator != (const Rect& rc) const
	{
		return !(*this == rc);
	}
};

}

