#pragma once

#include <string>
#include "Ui/Rect.h"
#include "Ui/Enums.h"
#include "Core/Math/Color4ub.h"

namespace traktor
{

class OutputStream;

	namespace ui
	{

NSString* makeNSString(const std::wstring& str);

std::wstring fromNSString(const NSString* str);

NSColor* makeNSColor(const Color4ub& color);

NSSize makeNSSize(const Size& sz);

NSPoint makeNSPoint(const Point& pt);

NSRect makeNSRect(const Rect& rc);

NSRect makeNSRect(const NSPoint& pt);

NSRect flipNSRect(const NSRect& rc);

Size fromNSSize(const NSSize& sz);

Point fromNSPoint(const NSPoint& pt);

Rect fromNSRect(const NSRect& rc);

OutputStream& formatNSSize(OutputStream& os, const NSSize& sz);

OutputStream& formatNSPoint(OutputStream& os, const NSPoint& pt);

OutputStream& formatNSRect(OutputStream& os, const NSRect& rc);

VirtualKey translateKeyCode(NSUInteger keyCode);

	}
}

