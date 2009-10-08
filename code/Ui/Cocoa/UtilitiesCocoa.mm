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

NSColor* makeNSColor(const Color& color)
{
	return [NSColor
		colorWithCalibratedRed:(color.r / 255.0f)
		green:(color.g / 255.0f)
		blue:(color.b / 255.0f)
		alpha:(1.0f)
	];
}

NSPoint makeNSPoint(NSView* view, const Point& pt)
{
	NSRect bounds = [view bounds];
	NSPoint npt = NSMakePoint(pt.x, pt.y);
	return npt;
}

NSRect makeNSRect(NSView* view, const Rect& rc)
{
	NSRect bounds = [view bounds];
	NSRect nrc = NSMakeRect(
		rc.left,
		rc.top,
		rc.getSize().cx,
		rc.getSize().cy
	);
	return nrc;
}

Point fromNSPoint(NSView* view, const NSPoint& pt)
{
	NSRect bounds = [view bounds];
	Point tpt(
		pt.x,
		pt.y
	);
	return tpt;
}

Rect fromNSRect(NSView* view, const NSRect& rc)
{
	NSRect bounds = [view bounds];
	Rect trc(
		Point(
			rc.origin.x,
			rc.origin.y
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
