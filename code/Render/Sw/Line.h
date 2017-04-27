/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_Line_H
#define traktor_render_Line_H

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup SW
 */
template < typename VisitorType, typename UserDataType >
void line(
	const Vector2* v,
	VisitorType* visitor,
	void (VisitorType::*visitMethod)(const UserDataType& userData, int x, int y, float d),
	const UserDataType& userData
)
{
	float dx = v[1].x - v[0].x;
	float dy = v[1].y - v[0].y;

	float d = std::sqrt(dx * dx + dy * dy);
	dx /= d;
	dy /= d;

	float x = v[0].x;
	float y = v[0].y;

	for (int i = 0; i <= int(d); ++i)
	{
		(visitor->*visitMethod)(userData, int(x), int(y), i / d);
		x += dx;
		y += dy;
	}
}

	}
}

#endif	// traktor_render_Line_H
