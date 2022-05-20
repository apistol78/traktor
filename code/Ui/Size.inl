#include "Ui/Size.h"

namespace traktor
{
	namespace ui
	{

inline Size::Size()
:	cx(0)
,	cy(0)
{
}

inline Size::Size(int32_t x, int32_t y)
:	cx(x)
,	cy(y)
{
}

inline Size::Size(const Size& size)
:	cx(size.cx)
,	cy(size.cy)
{
}

inline Size Size::operator - () const
{
	return Size(-cx, -cy);
}

inline Size Size::operator + (const Size& r) const
{
	return Size(cx + r.cx, cy + r.cy);
}

inline Size& Size::operator += (const Size& r)
{
	cx += r.cx;
	cy += r.cy;
	return *this;
}

inline Size Size::operator - (const Size& r) const
{
	return Size(cx - r.cx, cy - r.cy);
}

inline Size& Size::operator -= (const Size& r)
{
	cx -= r.cx;
	cy -= r.cy;
	return *this;
}

inline bool Size::operator == (const Size& r) const
{
	return cx == r.cx && cy == r.cy;
}

inline bool Size::operator != (const Size& r) const
{
	return cx != r.cx || cy != r.cy;
}

	}
}
