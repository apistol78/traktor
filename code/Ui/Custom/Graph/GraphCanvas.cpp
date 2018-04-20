#include "Ui/Canvas.h"
#include "Ui/Custom/Graph/GraphCanvas.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

Size operator * (const Size& sz, float scale)
{
	return Size(
		(int32_t)std::floor(sz.cx * scale + 0.5f),
		(int32_t)std::floor(sz.cy * scale + 0.5f)
	);
}

Point operator * (const Point& pt, float scale)
{
	return Point(
		(int32_t)std::floor(pt.x * scale),
		(int32_t)std::floor(pt.y * scale)
	);
}

Rect operator * (const Rect& rc, float scale)
{
	return Rect(
		rc.getTopLeft() * scale,
		rc.getSize() * scale
	);
}

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GraphCanvas", GraphCanvas, Object)

GraphCanvas::GraphCanvas(Canvas* canvas, const PaintSettings* paintSettings, float scale)
:	m_canvas(canvas)
,	m_paintSettings(paintSettings)
,	m_scale(scale)
{
}

void GraphCanvas::setForeground(const Color4ub& foreground)
{
	m_canvas->setForeground(foreground);
}

void GraphCanvas::setBackground(const Color4ub& background)
{
	m_canvas->setBackground(background);
}

void GraphCanvas::setFont(const Font& font)
{
	m_originalFont = font;

	m_scaledFont = font;
	m_scaledFont.setSize((int32_t)std::ceil(font.getSize() * m_scale));

	m_canvas->setFont(m_scaledFont);
}

void GraphCanvas::drawLine(const Point& start, const Point& end)
{
	m_canvas->drawLine(
		start * m_scale,
		end * m_scale
	);
}

void GraphCanvas::drawLines(const std::vector< Point >& pnts, int32_t thickness)
{
	std::vector< Point > tpnts; tpnts.reserve(pnts.size());
	for (std::vector< Point >::const_iterator i = pnts.begin(); i != pnts.end(); ++i)
		tpnts.push_back(*i * m_scale);

	m_canvas->setPenThickness((int32_t)std::ceil(thickness * m_scale));
	m_canvas->drawLines(tpnts);
	m_canvas->setPenThickness(1);
}

void GraphCanvas::drawRect(const Rect& rc)
{
	m_canvas->drawRect(rc * m_scale);
}

void GraphCanvas::fillRect(const Rect& rc)
{
	m_canvas->fillRect(rc * m_scale);
}

void GraphCanvas::fillPolygon(const Point* pnts, int count)
{
	std::vector< Point > tpnts; tpnts.reserve(count);
	for (int i = 0; i < count; ++i)
		tpnts.push_back(pnts[i] * m_scale);
	m_canvas->fillPolygon(&tpnts[0], count);
}

void GraphCanvas::drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, uint32_t blendMode)
{
	m_canvas->drawBitmap(
		dstAt * m_scale,
		dstSize * m_scale,
		srcAt,
		srcSize,
		bitmap,
		blendMode
	);
}

void GraphCanvas::drawText(const Point& at, const std::wstring& text)
{
	m_canvas->drawText(
		at * m_scale,
		text
	);
}

void GraphCanvas::drawText(const Rect& rc, const std::wstring& text, Align halign, Align valign)
{
	m_canvas->drawText(
		rc * m_scale,
		text,
		halign,
		valign
	);
}

Size GraphCanvas::getTextExtent(const std::wstring& text) const
{
	m_canvas->setFont(m_originalFont);
	Size sz = m_canvas->getTextExtent(text);
	m_canvas->setFont(m_scaledFont);
	return sz;
}

		}
	}
}
