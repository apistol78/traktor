#import <Cocoa/Cocoa.h>
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/Point.h"
#include "Ui/Rect.h"
#include "Ui/Cocoa/UtilitiesCocoa.h"

namespace traktor
{
	namespace ui
	{

NSString* makeNSString(const std::wstring& str)
{
	std::string mbs = wstombs(Utf8Encoding(), str);
	return [[[NSString alloc] initWithCString: mbs.c_str() encoding: NSUTF8StringEncoding] autorelease];
}

std::wstring fromNSString(const NSString* str)
{
	char buffer[4096];
	[str getCString: buffer maxLength: sizeof_array(buffer) encoding: NSUTF8StringEncoding];
	return mbstows(buffer);
}

NSColor* makeNSColor(const Color4ub& color)
{
	return [NSColor
		colorWithCalibratedRed:(color.r / 255.0f)
		green:(color.g / 255.0f)
		blue:(color.b / 255.0f)
		alpha:(color.a / 255.0f)
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

VirtualKey translateKeyCode(NSUInteger keyCode)
{
	const struct { NSUInteger keyCode; VirtualKey vkey; } c_translateTable[] =
	{
		//{ 0, VkNull },
		//{ 0, VkBack },
		{ 48, VkTab },
		//{ 0, VkClear },
		{ 36, VkReturn },
		{ 56, VkShift },
		{ 59, VkControl },
		{ 58, VkMenu },
		//{ 0, VkPause },
		//{ 0, VkCapital },
		{ 53, VkEscape },
		{ 49, VkSpace },
		{ 119, VkEnd },
		{ 115, VkHome },
		{ 116, VkPageUp },
		{ 121, VkPageDown },
		{ 123, VkLeft },
		{ 126, VkUp },
		{ 124, VkRight },
		{ 125, VkDown },
		//{ 0, VkSelect },
		//{ 0, VkPrint },
		//{ 0, VkExecute },
		//{ 0, VkSnapshot },
		//{ 0, VkInsert },
		//{ 0, VkDelete },
		{ 51, VkBackSpace },
		//{ 0, VkHelp },
		{ 82, VkNumPad0 },
		{ 83, VkNumPad1 },
		{ 84, VkNumPad2 },
		{ 85, VkNumPad3 },
		{ 86, VkNumPad4 },
		{ 87, VkNumPad5 },
		{ 88, VkNumPad6 },
		{ 89, VkNumPad7 },
		{ 91, VkNumPad8 },
		{ 92, VkNumPad9 },
		{ 67, VkMultiply },
		{ 69, VkAdd },
		{ 81, VkSeparator },
		{ 78, VkSubtract },
		{ 65, VkDecimal },
		{ 75, VkDivide },
		{ 122, VkF1 },
		{ 120, VkF2 },
		{ 99, VkF3 },
		{ 118, VkF4 },
		{ 96, VkF5 },
		{ 97, VkF6 },
		{ 98, VkF7 },
		{ 100, VkF8 },
		{ 101, VkF9 },
		{ 109, VkF10 },
		{ 103, VkF11 },
		{ 111, VkF12 },
		//{ 0, VkNumLock },
		//{ 0, VkScroll },
		
		{ 0, VkA },
		{ 11, VkB },
		{ 8, VkC },
		{ 2, VkD },
		{ 14, VkE },
		{ 3, VkF },
		{ 5, VkG },
		{ 4, VkH },
		{ 34, VkI },
		{ 38, VkJ },
		{ 40, VkK },
		{ 37, VkL },
		{ 46, VkM },
		{ 45, VkN },
		{ 31, VkO },
		{ 35, VkP },
		{ 12, VkQ },
		{ 15, VkR },
		{ 1, VkS },
		{ 17, VkT },
		{ 32, VkU },
		{ 9, VkV },
		{ 13, VkW },
		{ 7, VkX },
		{ 16, VkY },
		{ 6, VkZ },
		{ 29, Vk0 },
		{ 18, Vk1 },
		{ 19, Vk2 },
		{ 20, Vk3 },
		{ 21, Vk4 },
		{ 23, Vk5 },
		{ 22, Vk6 },
		{ 26, Vk7 },
		{ 28, Vk8 },
		{ 25, Vk9 }
	};
	
	for (int i = 0; i < sizeof_array(c_translateTable); ++i)
	{
		if (c_translateTable[i].keyCode == keyCode)
			return c_translateTable[i].vkey;
	}
	
	return VkNull;
}

	}
}
