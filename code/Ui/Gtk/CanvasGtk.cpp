#include "Core/Math/Const.h"
#include "Core/Misc/TString.h"
#include "Ui/Gtk/BitmapGtk.h"
#include "Ui/Gtk/CanvasGtk.h"

namespace traktor
{
	namespace ui
	{

CanvasGtk::CanvasGtk(const Cairo::RefPtr< Cairo::Context >& context)
:	m_context(context)
,	m_foreground(0, 0, 0, 255)
,	m_background(255, 255, 255, 255)
{
}

CanvasGtk::~CanvasGtk()
{
}

void CanvasGtk::setForeground(const Color4ub& foreground)
{
	m_foreground = foreground;
}

void CanvasGtk::setBackground(const Color4ub& background)
{
	m_background = background;
}

void CanvasGtk::setFont(const Font& font)
{
	m_context->select_font_face(
		wstombs(font.getFace()),
		Cairo::FONT_SLANT_NORMAL,
		font.isBold() ? Cairo::FONT_WEIGHT_BOLD : Cairo::FONT_WEIGHT_NORMAL
	);
	m_context->set_font_size(font.getSize());
}

void CanvasGtk::setLineStyle(LineStyle lineStyle)
{
}

void CanvasGtk::setPenThickness(int thickness)
{
	m_context->set_line_width(thickness);
}

void CanvasGtk::setClipRect(const Rect& rc)
{
}

void CanvasGtk::resetClipRect()
{
}

void CanvasGtk::drawPixel(int x, int y, const Color4ub& c)
{
}

void CanvasGtk::drawLine(int x1, int y1, int x2, int y2)
{
	m_context->set_source_rgba(m_foreground.e[0] / 255.0, m_foreground.e[1] / 255.0, m_foreground.e[2] / 255.0, m_foreground.e[3] / 255.0);
	m_context->move_to(x1, y1);
	m_context->line_to(x2, y2);
	m_context->stroke();
}

void CanvasGtk::drawLines(const Point* pnts, int npnts)
{
	m_context->set_source_rgba(m_foreground.e[0] / 255.0, m_foreground.e[1] / 255.0, m_foreground.e[2] / 255.0, m_foreground.e[3] / 255.0);
	m_context->move_to(pnts[0].x, pnts[0].y);
	for (int i = 1; i < npnts; ++i)
		m_context->line_to(pnts[i].x, pnts[1].y);
	m_context->stroke();
}

void CanvasGtk::fillCircle(int x, int y, float radius)
{
	m_context->set_source_rgba(m_background.e[0] / 255.0, m_background.e[1] / 255.0, m_background.e[2] / 255.0, m_background.e[3] / 255.0);
	m_context->arc(x, y, radius, 0.0, TWO_PI);
	m_context->fill();
}

void CanvasGtk::drawCircle(int x, int y, float radius)
{
	m_context->set_source_rgba(m_foreground.e[0] / 255.0, m_foreground.e[1] / 255.0, m_foreground.e[2] / 255.0, m_foreground.e[3] / 255.0);
	m_context->arc(x, y, radius, 0.0, TWO_PI);
	m_context->stroke();
}

void CanvasGtk::drawEllipticArc(int x, int y, int w, int h, float start, float end)
{
}

void CanvasGtk::drawSpline(const Point* pnts, int npnts)
{
}

void CanvasGtk::fillRect(const Rect& rc)
{
	m_context->set_source_rgba(m_background.e[0] / 255.0, m_background.e[1] / 255.0, m_background.e[2] / 255.0, m_background.e[3] / 255.0);
	m_context->rectangle(rc.left, rc.top, rc.getWidth(), rc.getHeight());
	m_context->fill();
}

void CanvasGtk::fillGradientRect(const Rect& rc, bool vertical)
{
	m_context->set_source_rgba(m_background.e[0] / 255.0, m_background.e[1] / 255.0, m_background.e[2] / 255.0, m_background.e[3] / 255.0);
	m_context->rectangle(rc.left, rc.top, rc.getWidth(), rc.getHeight());
	m_context->fill();
}

void CanvasGtk::drawRect(const Rect& rc)
{
	m_context->set_source_rgba(m_foreground.e[0] / 255.0, m_foreground.e[1] / 255.0, m_foreground.e[2] / 255.0, m_foreground.e[3] / 255.0);
	m_context->rectangle(rc.left, rc.top, rc.getWidth(), rc.getHeight());
	m_context->stroke();
}

void CanvasGtk::drawRoundRect(const Rect& rc, int radius)
{
	m_context->set_source_rgba(m_foreground.e[0] / 255.0, m_foreground.e[1] / 255.0, m_foreground.e[2] / 255.0, m_foreground.e[3] / 255.0);
	m_context->rectangle(rc.left, rc.top, rc.getWidth(), rc.getHeight());
	m_context->stroke();
}

void CanvasGtk::drawPolygon(const Point* pnts, int count)
{
	m_context->set_source_rgba(m_foreground.e[0] / 255.0, m_foreground.e[1] / 255.0, m_foreground.e[2] / 255.0, m_foreground.e[3] / 255.0);
	m_context->move_to(pnts[0].x, pnts[0].y);
	for (int i = 1; i < count; ++i)
		m_context->line_to(pnts[i].x, pnts[1].y);
	m_context->line_to(pnts[0].x, pnts[0].y);
	m_context->stroke();
}

void CanvasGtk::fillPolygon(const Point* pnts, int count)
{
	m_context->set_source_rgba(m_foreground.e[0] / 255.0, m_foreground.e[1] / 255.0, m_foreground.e[2] / 255.0, m_foreground.e[3] / 255.0);
	m_context->move_to(pnts[0].x, pnts[0].y);
	for (int i = 1; i < count; ++i)
		m_context->line_to(pnts[i].x, pnts[1].y);
	m_context->line_to(pnts[0].x, pnts[0].y);
	m_context->fill();
}

void CanvasGtk::drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, IBitmap* bitmap, uint32_t blendMode)
{
	BitmapGtk* bm = static_cast< BitmapGtk* >(bitmap);
	T_ASSERT (bm);

	const Cairo::RefPtr< Cairo::ImageSurface >& surface = bm->getSurface();

	m_context->set_source(surface, srcAt.x, srcAt.y);
	m_context->rectangle(dstAt.x, dstAt.y, size.cx, size.cy);
	m_context->fill();
}

void CanvasGtk::drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, uint32_t blendMode)
{
	BitmapGtk* bm = static_cast< BitmapGtk* >(bitmap);
	T_ASSERT (bm);

	const Cairo::RefPtr< Cairo::ImageSurface >& surface = bm->getSurface();

	m_context->set_source(surface, srcAt.x, srcAt.y);
	m_context->rectangle(dstAt.x, dstAt.y, dstSize.cx, dstSize.cy);
	m_context->fill();
}

void CanvasGtk::drawText(const Point& at, const std::wstring& text)
{
	m_context->set_source_rgba(m_foreground.e[0] / 255.0, m_foreground.e[1] / 255.0, m_foreground.e[2] / 255.0, m_foreground.e[3] / 255.0);
	m_context->move_to(at.x, at.y);
	m_context->show_text(wstombs(text));
}

void CanvasGtk::drawText(const Rect& rc, const std::wstring& text, Align halign, Align valign)
{
	m_context->set_source_rgba(m_foreground.e[0] / 255.0, m_foreground.e[1] / 255.0, m_foreground.e[2] / 255.0, m_foreground.e[3] / 255.0);
	m_context->move_to(rc.left, rc.top);
	m_context->show_text(wstombs(text));
}

Size CanvasGtk::getTextExtent(const std::wstring& text) const
{
	Cairo::TextExtents te;
	m_context->get_text_extents(wstombs(text), te);
	return Size(int32_t(te.width + 0.5), int32_t(te.height + 0.5));
}

void* CanvasGtk::getSystemHandle()
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
