/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_Rect_H
#define traktor_ui_Rect_H

#include "Ui/Point.h"
#include "Ui/Size.h"

namespace traktor
{
	namespace ui
	{

/*! \brief Rectangle
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
	
	inline Rect& operator = (const Rect& rc);

	inline bool operator == (const Rect& rc) const;

	inline bool operator != (const Rect& rc) const;
};

	}
}

#include "Ui/Rect.inl"

#endif	// traktor_ui_Rect_H
