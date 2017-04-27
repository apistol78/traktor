/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Ui/Rect.h"

namespace traktor
{
	namespace ui
	{

inline Rect::Rect()
:	left(0)
,	top(0)
,	right(0)
,	bottom(0)
{
}

inline Rect::Rect(int32_t l, int32_t t, int32_t r, int32_t b)
:	left(l)
,	top(t)
,	right(r)
,	bottom(b)
{
}

inline Rect::Rect(const Point& tl, const Point& br)
:	left(tl.x)
,	top(tl.y)
,	right(br.x)
,	bottom(br.y)
{
}

inline Rect::Rect(const Point& tl, const Size& wh)
:	left(tl.x)
,	top(tl.y)
,	right(tl.x + wh.cx)
,	bottom(tl.y + wh.cy)
{
}

inline Rect::Rect(const Rect& rc)
:	left(rc.left)
,	top(rc.top)
,	right(rc.right)
,	bottom(rc.bottom)
{
}

inline void Rect::moveTo(const Point& pnt)
{
	Size size(getSize());
	left = pnt.x;
	top = pnt.y;
	right = pnt.x + size.cx;
	bottom = pnt.y + size.cy;
}

inline void Rect::setWidth(int32_t width)
{
	right = left + width;
}

inline int32_t Rect::getWidth() const
{
	return right - left;
}

inline void Rect::setHeight(int32_t height)
{
	bottom = top + height;
}

inline int32_t Rect::getHeight() const
{
	return bottom - top;
}

inline void Rect::setSize(const Size& size)
{
	setWidth(size.cx);
	setHeight(size.cy);
}

inline Size Rect::getSize() const
{
	return Size(getWidth(), getHeight());
}

inline Point Rect::getTopLeft() const
{
	return Point(left, top);
}

inline Point Rect::getTopRight() const
{
	return Point(right, top);
}

inline Point Rect::getBottomLeft() const
{
	return Point(left, bottom);
}

inline Point Rect::getBottomRight() const
{
	return Point(right, bottom);
}

inline Point Rect::getCenter() const
{
	return Point((left + right) / 2, (top + bottom) / 2);
}

inline Rect Rect::getUnified() const
{
	Rect rc = *this;
	if (rc.left > rc.right)
		std::swap(rc.left, rc.right);
	if (rc.top > rc.bottom)
		std::swap(rc.top, rc.bottom);
	return rc;
}

inline Rect Rect::offset(const Size& sz) const
{
	return offset(sz.cx, sz.cy);
}

inline Rect Rect::offset(int32_t x, int32_t y) const
{
	return Rect(left + x, top + y, right + x, bottom + y);
}

inline Rect Rect::inflate(const Size& sz) const
{
	return inflate(sz.cx, sz.cy);
}

inline Rect Rect::inflate(int32_t x, int32_t y) const
{
	return Rect(left - x, top - y, right + x, bottom + y);
}

inline int32_t Rect::area() const
{
	return getWidth() * getHeight();
}

inline Rect Rect::contain(const Point& pnt) const
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

inline bool Rect::inside(const Point& pnt, bool inclusive) const
{
	return
		inclusive ?
		bool(pnt.x >= left && pnt.x <= right && pnt.y >= top && pnt.y <= bottom) :
		bool(pnt.x > left && pnt.x < right && pnt.y > top && pnt.y < bottom);
}

inline bool Rect::intersect(const Rect& rc) const
{
	return (left < rc.right) && (top < rc.bottom) && (right > rc.left) && (bottom > rc.top);
}

inline Rect& Rect::operator = (const Rect& rc)
{
	left = rc.left;
	top = rc.top;
	right = rc.right;
	bottom = rc.bottom;
	return *this;
}

inline bool Rect::operator == (const Rect& rc) const
{
	return
		left == rc.left &&
		top == rc.top &&
		right == rc.right &&
		bottom == rc.bottom;
}

inline bool Rect::operator != (const Rect& rc) const
{
	return !(*this == rc);
}

	}
}
