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
template < typename ValueType >
class RectBase
{
public:
	typedef PointBase< ValueType > point_t;
	typedef SizeBase< ValueType > size_t;

	ValueType left = ValueType(0);
	ValueType top = ValueType(0);
	ValueType right = ValueType(0);
	ValueType bottom = ValueType(0);

	RectBase() = default;

	RectBase(ValueType l, ValueType t, ValueType r, ValueType b)
	:	left(l)
	,	top(t)
	,	right(r)
	,	bottom(b)
	{
	}

	RectBase(const point_t& tl, const point_t& br)
	:	left(tl.x)
	,	top(tl.y)
	,	right(br.x)
	,	bottom(br.y)
	{
	}

	RectBase(const point_t& tl, const size_t& wh)
	:	left(tl.x)
	,	top(tl.y)
	,	right(tl.x + wh.cx)
	,	bottom(tl.y + wh.cy)
	{
	}

	RectBase(const RectBase< ValueType >& rc)
	:	left(rc.left)
	,	top(rc.top)
	,	right(rc.right)
	,	bottom(rc.bottom)
	{
	}

	void moveTo(const point_t& pnt)
	{
		const size_t size = getSize();
		left = pnt.x;
		top = pnt.y;
		right = pnt.x + size.cx;
		bottom = pnt.y + size.cy;
	}

	void setWidth(ValueType width)
	{
		right = left + width;
	}

	ValueType getWidth() const
	{
		return right - left;
	}

	void setHeight(ValueType height)
	{
		bottom = top + height;
	}

	ValueType getHeight() const
	{
		return bottom - top;
	}

	void setSize(const size_t& size)
	{
		setWidth(size.cx);
		setHeight(size.cy);
	}

	size_t getSize() const
	{
		return size_t(right - left, bottom - top);
	}

	point_t getTopLeft() const
	{
		return point_t(left, top);
	}

	point_t getTopRight() const
	{
		return point_t(right, top);
	}

	point_t getBottomLeft() const
	{
		return point_t(left, bottom);
	}

	point_t getBottomRight() const
	{
		return point_t(right, bottom);
	}

	point_t getCenter() const
	{
		return point_t((left + right) / ValueType(2), (top + bottom) / ValueType(2));
	}

	RectBase< ValueType > getUnified() const
	{
		RectBase< ValueType > rc = *this;
		if (rc.left > rc.right)
			std::swap(rc.left, rc.right);
		if (rc.top > rc.bottom)
			std::swap(rc.top, rc.bottom);
		return rc;
	}

	RectBase< ValueType > offset(const size_t& sz) const
	{
		return offset(sz.cx, sz.cy);
	}

	RectBase< ValueType > offset(ValueType x, ValueType y) const
	{
		return RectBase< ValueType >(left + x, top + y, right + x, bottom + y);
	}

	RectBase< ValueType > inflate(const size_t& sz) const
	{
		return inflate(sz.cx, sz.cy);
	}

	RectBase< ValueType > inflate(ValueType x, ValueType y) const
	{
		return RectBase< ValueType >(left - x, top - y, right + x, bottom + y);
	}

	ValueType area() const
	{
		return (right - left) * (bottom - top);
	}

	RectBase< ValueType > contain(const point_t& pnt) const
	{
		RectBase< ValueType > rc = *this;
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

	RectBase< ValueType > contain(const RectBase< ValueType >& rc) const
	{
		return
			contain(rc.getTopLeft()).
			contain(rc.getTopRight()).
			contain(rc.getBottomLeft()).
			contain(rc.getBottomRight());
	}

	bool inside(const point_t& pnt, bool inclusive = true) const
	{
		return
			inclusive ?
			bool(pnt.x >= left && pnt.x <= right && pnt.y >= top && pnt.y <= bottom) :
			bool(pnt.x > left && pnt.x < right && pnt.y > top && pnt.y < bottom);
	}

	bool intersect(const RectBase< ValueType >& rc) const
	{
		return (left < rc.right) && (top < rc.bottom) && (right > rc.left) && (bottom > rc.top);
	}

	RectBase< ValueType >& operator = (const RectBase< ValueType >& rc)
	{
		left = rc.left;
		top = rc.top;
		right = rc.right;
		bottom = rc.bottom;
		return *this;
	}

	bool operator == (const RectBase< ValueType >& rc) const
	{
		return
			left == rc.left &&
			top == rc.top &&
			right == rc.right &&
			bottom == rc.bottom;
	}

	bool operator != (const RectBase< ValueType >& rc) const
	{
		return !(*this == rc);
	}
};

typedef RectBase< int32_t > Rect;
typedef RectBase< Unit > UnitRect;

}

