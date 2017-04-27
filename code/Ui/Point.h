/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_Point_H
#define traktor_ui_Point_H

#include "Ui/Size.h"

namespace traktor
{
	namespace ui
	{

#pragma pack(1)

/*! \brief 2D point.
 * \ingroup UI
 */
class Point
{
public:
	int x;
	int y;

	inline Point();

	inline Point(const Point& pnt);

	inline Point(int x_, int y_);

	inline Point operator + (const Size& s) const;

	inline Point operator - (const Size& s) const;

	inline const Point& operator += (const Size& s);

	inline const Point& operator -= (const Size& s);

	inline Size operator - (const Point& p) const;

	inline bool operator == (const Point& p) const;

	inline bool operator != (const Point& p) const;
};

#pragma pack()

	}
}

#include "Ui/Point.inl"

#endif	// traktor_ui_Point_H
