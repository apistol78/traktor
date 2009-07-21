#ifndef traktor_ui_xtrme_CanvasXtrme_H
#define traktor_ui_xtrme_CanvasXtrme_H

#include <map>
#include "Core/Heap/Ref.h"
#include "Ui/Itf/ICanvas.h"
#include "Ui/Xtrme/BatchRenderer.h"
#include "Ui/Xtrme/ImageCache.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class IRenderView;

	}

	namespace ui
	{
		namespace xtrme
		{

class FontMap;

class CanvasXtrme : public ICanvas
{
public:
	CanvasXtrme(render::IRenderSystem* renderSystem, render::IRenderView* renderView, const Font& font);

	virtual void setForeground(const Color& foreground);

	virtual void setBackground(const Color& background);

	virtual void setFont(const Font& font);

	virtual void setLineStyle(LineStyle lineStyle);

	virtual void setPenThickness(int thickness);

	virtual void setClipRect(const Rect& rc);

	virtual void resetClipRect();
	
	virtual void drawPixel(int x, int y, const Color& c);

	virtual void drawLine(int x1, int y1, int x2, int y2);

	virtual void drawLines(const Point* pnts, int npnts);

	virtual void fillCircle(int x, int y, float radius);

	virtual void drawCircle(int x, int y, float radius);

	virtual void drawEllipticArc(int x, int y, int w, int h, float start, float end);

	virtual void drawSpline(const Point* pnts, int npnts);

	virtual void fillRect(const Rect& rc);

	virtual void fillGradientRect(const Rect& rc, bool vertical);

	virtual void drawRect(const Rect& rc);

	virtual void drawRoundRect(const Rect& rc, int radius);

	virtual void drawPolygon(const Point* pnts, int count);

	virtual void fillPolygon(const Point* pnts, int count);

	virtual void drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, IBitmap* bitmap, BlendMode blendMode);

	virtual void drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, BlendMode blendMode);

	virtual void drawText(const Point& at, const std::wstring& text);

	virtual void drawText(const Rect& rc, const std::wstring& text, Align halign, Align valign);
	
	virtual Size getTextExtent(const std::wstring& text) const;

	void begin();

	void end(const Size& size);

private:
	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::IRenderView > m_renderView;
	BatchRenderer m_batchRenderer;
	ImageCache m_imageCache;
	std::map< Font, Ref< FontMap > > m_fontMaps;
	Ref< FontMap > m_currentFontMap;
	Color m_backGround;
	Color m_foreGround;
	int m_thickness;

	void drawSubLine(int x1, int y1, int x2, int y2);
};

		}
	}
}

#endif	// traktor_ui_xtrme_CanvasXtrme_H
