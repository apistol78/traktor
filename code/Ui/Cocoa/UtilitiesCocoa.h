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

struct NSColor* makeNSColor(const Color& color);

NSPoint makeNSPoint(NSView* view, const Point& pt);

NSRect makeNSRect(NSView* view, const Rect& rc);

Point fromNSPoint(NSView* view, const NSPoint& pt);

Rect fromNSRect(NSView* view, const NSRect& rc);

	}
}

#endif	// traktor_ui_UtilitiesCocoa_H
