#ifndef traktor_ui_Canvas_H
#define traktor_ui_Canvas_H

#include "Core/Object.h"
#include "Ui/Itf/ICanvas.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Bitmap;
	
/*! \brief Canvas
 * \ingroup UI
 */
class T_DLLCLASS Canvas : public Object
{
	T_RTTI_CLASS(Canvas)

public:
	Canvas(ICanvas* canvas);

	void setForeground(const Color& foreground);

	void setBackground(const Color& background);

	void setFont(const Font& font);

	void setLineStyle(LineStyle lineStyle);

	void setPenThickness(int thickness);

	void setClipRect(const Rect& rc);

	void resetClipRect();
	
	void drawPixel(int x, int y);

	void drawPixel(const Point& p);

	void drawPixel(int x, int y, const Color& c);

	void drawPixel(const Point& p, const Color& c);

	void drawLine(int x1, int y1, int x2, int y2);

	void drawLine(const Point& start, const Point& end);
	
	void drawLines(const std::vector< Point >& pnts);
	
	void drawLines(const Point* pnts, int npnts);

	void fillCircle(const Point& p, float radius);

	void drawCircle(const Point& p, float radius);

	void drawEllipticArc(const Point& p, const Size& s, float start, float end);

	void drawSpline(const std::vector< Point >& pnts);

	void drawSpline(const Point* pnts, int npnts);

	void fillRect(const Rect& rc);

	void fillGradientRect(const Rect& rc, bool vertical = true);

	void drawRect(const Rect& rc);

	void drawRoundRect(const Rect& rc, int radius);

	void drawPolygon(const Point* pnts, int count);

	void fillPolygon(const Point* pnts, int count);

	void drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, Bitmap* bitmap, BlendMode blendMode = BmNone);

	void drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, Bitmap* bitmap, BlendMode blendMode = BmNone);

	void drawText(const Point& at, const std::wstring& text);

	void drawText(const Rect& rc, const std::wstring& text, Align halign = AnLeft, Align valign = AnTop);
	
	Size getTextExtent(const std::wstring& text) const;

private:
	ICanvas* m_canvas;
	Color m_foreground;
	Color m_background;
};
	
	}
}

#endif	// traktor_ui_Canvas_H
