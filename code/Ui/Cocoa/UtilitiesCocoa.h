#ifndef traktor_ui_UtilitiesCocoa_H
#define traktor_ui_UtilitiesCocoa_H

#include <string>
#include "Ui/Rect.h"
#include "Core/Math/Color.h"

namespace traktor
{
	namespace ui
	{

struct NSString* makeNSString(const std::wstring& str);

std::wstring fromNSString(const NSString* str);

struct NSColor* makeNSColor(const Color& color);

NSSize makeNSSize(const Size& sz);

NSPoint makeNSPoint(const Point& pt);

NSRect makeNSRect(const Rect& rc);

Size fromNSSize(const NSSize& sz);

Point fromNSPoint(const NSPoint& pt);

Rect fromNSRect(const NSRect& rc);

	}
}

#endif	// traktor_ui_UtilitiesCocoa_H
