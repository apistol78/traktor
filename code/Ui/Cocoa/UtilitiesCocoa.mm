#import <Cocoa/Cocoa.h>

#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Point.h"
#include "Ui/Rect.h"
#include "Core/Misc/TString.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{
		
OutputStream& operator << (OutputStream& os, const NSPoint& npt)
{
	os << npt.x << L", " << npt.y;
	return os;
}

OutputStream& operator << (OutputStream& os, const NSRect& nrc)
{
	os << nrc.origin.x << L", " << nrc.origin.y << L" - " << nrc.origin.x + nrc.size.width << L", " << nrc.origin.y + nrc.size.height;
	return os;
}

OutputStream& operator << (OutputStream& os, const Point& pt)
{
	os << pt.x << L", " << pt.y;
	return os;
}

OutputStream& operator << (OutputStream& os, const Rect& rc)
{
	os << rc.left << L", " << rc.top << L" - " << rc.right << L", " << rc.bottom;
	return os;
}

		}

NSString* makeNSString(const std::wstring& str)
{
	std::string mbs = wstombs(str);
	return [[NSString alloc] initWithCString: mbs.c_str() encoding: NSUTF8StringEncoding];
}

std::wstring fromNSString(const NSString* str)
{
	char buffer[4096];
	[str getCString: buffer maxLength: sizeof_array(buffer) encoding: NSUTF8StringEncoding];
	return mbstows(buffer);
}

NSColor* makeNSColor(const Color& color)
{
	return [NSColor
		colorWithCalibratedRed:(color.r / 255.0f)
		green:(color.g / 255.0f)
		blue:(color.b / 255.0f)
		alpha:(1.0f)
	];
}

NSSize makeNSSize(const Size& sz)
{
	return NSMakeSize(sz.cx, sz.cy);
}

NSPoint makeNSPoint(const Point& pt)
{
	return NSMakePoint(pt.x, pt.y);
}

NSRect makeNSRect(const Rect& rc)
{
	NSRect nrc = NSMakeRect(
		rc.left,
		rc.top,
		rc.getSize().cx,
		rc.getSize().cy
	);
	return nrc;
}

Size fromNSSize(const NSSize& sz)
{
	return Size(
		int32_t(sz.width),
		int32_t(sz.height)
	);
}

Point fromNSPoint(const NSPoint& pt)
{
	Point tpt(
		int32_t(pt.x),
		int32_t(pt.y)
	);
	return tpt;
}

Rect fromNSRect(const NSRect& rc)
{
	Rect trc(
		Point(
			int32_t(rc.origin.x),
			int32_t(rc.origin.y)
		),
		Size(
			rc.size.width,
			rc.size.height
		)
	);
	return trc;
}

	}
}
