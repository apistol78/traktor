#import <Cocoa/Cocoa.h>

#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Point.h"
#include "Ui/Rect.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace ui
	{

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
	NSRect bounds = [view frame];
	return NSMakePoint(pt.x, pt.y); // bounds.size.height - pt.y);
}

NSRect makeNSRect(NSView* view, const Rect& rc)
{
	NSRect bounds = [view frame];
	return NSMakeRect(
		rc.left,
		/*bounds.size.height - */rc.top,
		rc.getSize().cx,
		rc.getSize().cy
	);
}

Point fromNSPoint(NSView* view, const NSPoint& pt)
{
	NSRect bounds = [view frame];
	return Point(
		pt.x,
		/*bounds.size.height - */pt.y
	);
}

Rect fromNSRect(NSView* view, const NSRect& rc)
{
	NSRect bounds = [view frame];
	return Rect(
		Point(
			rc.origin.x,
			/*bounds.size.height - */rc.origin.y
		),
		Size(
			rc.size.width,
			rc.size.height
		)
	);
}

	}
}
