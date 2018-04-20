#ifndef traktor_ui_custom_GraphCanvas_H
#define traktor_ui_custom_GraphCanvas_H

#include "Core/Object.h"
#include "Ui/Enums.h"
#include "Ui/Font.h"

namespace traktor
{

class Color4ub;

	namespace ui
	{

class Canvas;
class IBitmap;
class Point;
class Rect;
class Size;

		namespace custom
		{

class PaintSettings;

class GraphCanvas : public Object
{
	T_RTTI_CLASS;

public:
	GraphCanvas(Canvas* canvas, const PaintSettings* paintSettings, float scale);

	void setForeground(const Color4ub& foreground);

	void setBackground(const Color4ub& background);

	void setFont(const Font& font);

	void drawLine(const Point& start, const Point& end);

	void drawLines(const std::vector< Point >& pnts, int32_t thickness);

	void drawRect(const Rect& rc);

	void fillRect(const Rect& rc);

	void fillPolygon(const Point* pnts, int count);

	void drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, uint32_t blendMode);

	void drawText(const Rect& rc, const std::wstring& text, Align halign, Align valign);

	Size getTextExtent(const std::wstring& text) const;

	const PaintSettings* getPaintSettings() const { return m_paintSettings; }

private:
	Canvas* m_canvas;
	const PaintSettings* m_paintSettings;
	float m_scale;
	Font m_originalFont;
	Font m_scaledFont;
};

		}
	}
}

#endif	// traktor_ui_custom_GraphCanvas_H
