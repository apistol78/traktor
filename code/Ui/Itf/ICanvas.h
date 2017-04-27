/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ICanvas_H
#define traktor_ui_ICanvas_H

#include <string>
#include <vector>
#include "Core/Math/Color4ub.h"
#include "Ui/Enums.h"
#include "Ui/Font.h"
#include "Ui/Point.h"
#include "Ui/Rect.h"
#include "Ui/Size.h"

namespace traktor
{
	namespace ui
	{

class ISystemBitmap;

/*! \brief Canvas interface.
 * \ingroup UI
 */
class ICanvas
{
public:
	virtual void setForeground(const Color4ub& foreground) = 0;

	virtual void setBackground(const Color4ub& background) = 0;

	virtual void setFont(const Font& font) = 0;

	virtual void setLineStyle(LineStyle lineStyle) = 0;

	virtual void setPenThickness(int thickness) = 0;

	virtual void setClipRect(const Rect& rc) = 0;

	virtual void resetClipRect() = 0;
	
	virtual void drawPixel(int x, int y, const Color4ub& c) = 0;

	virtual void drawLine(int x1, int y1, int x2, int y2) = 0;

	virtual void drawLines(const Point* pnts, int npnts) = 0;

	virtual void fillCircle(int x, int y, float radius) = 0;

	virtual void drawCircle(int x, int y, float radius) = 0;

	virtual void drawEllipticArc(int x, int y, int w, int h, float start, float end) = 0;

	virtual void drawSpline(const Point* pnts, int npnts) = 0;

	virtual void fillRect(const Rect& rc) = 0;

	virtual void fillGradientRect(const Rect& rc, bool vertical) = 0;

	virtual void drawRect(const Rect& rc) = 0;

	virtual void drawRoundRect(const Rect& rc, int radius) = 0;

	virtual void drawPolygon(const Point* pnts, int count) = 0;

	virtual void fillPolygon(const Point* pnts, int count) = 0;

	virtual void drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, ISystemBitmap* bitmap, uint32_t blendMode) = 0;

	virtual void drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, ISystemBitmap* bitmap, uint32_t blendMode) = 0;

	virtual void drawText(const Point& at, const std::wstring& text) = 0;

	virtual void drawText(const Rect& rc, const std::wstring& text, Align halign, Align valign) = 0;
	
	virtual Size getTextExtent(const std::wstring& text) const = 0;

	virtual void* getSystemHandle() = 0;
};
	
	}
}

#endif	// traktor_ui_ICanvas_H
