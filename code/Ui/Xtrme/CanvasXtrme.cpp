#include <cmath>
#include "Ui/Xtrme/CanvasXtrme.h"
#include "Ui/Xtrme/FontMap.h"
#include "Ui/Itf/IBitmap.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Core/Math/Const.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Hermite.h"

namespace traktor
{
	namespace ui
	{
		namespace xtrme
		{
			namespace
			{

const int c_circleResolution = 16;

			}

CanvasXtrme::CanvasXtrme(render::IRenderSystem* renderSystem, render::IRenderView* renderView, const Font& font)
:	m_batchRenderer(renderSystem, renderView, 65536)
,	m_imageCache(renderSystem)
,	m_renderSystem(renderSystem)
,	m_renderView(renderView)
,	m_backGround(0, 0, 0)
,	m_foreGround(255, 255, 255)
,	m_thickness(1)
{
	setFont(font);
}

void CanvasXtrme::setForeground(const Color& foreground)
{
	m_foreGround = foreground;
}

void CanvasXtrme::setBackground(const Color& background)
{
	m_backGround = background;
}

void CanvasXtrme::setFont(const Font& font)
{
	std::map< const Font*, Ref< FontMap > >::iterator i = m_fontMaps.find(&font);
	if (i == m_fontMaps.end())
	{
		m_currentFontMap = new FontMap();
		m_currentFontMap->create(m_renderSystem, font);
		m_fontMaps[&font] = m_currentFontMap;
	}
	else
		m_currentFontMap = i->second;
}

void CanvasXtrme::setLineStyle(LineStyle lineStyle)
{
}

void CanvasXtrme::setPenThickness(int thickness)
{
	m_thickness = thickness;
}

void CanvasXtrme::setClipRect(const Rect& rc)
{
}

void CanvasXtrme::resetClipRect()
{
}

void CanvasXtrme::drawPixel(int x, int y, const Color& c)
{
}

void CanvasXtrme::drawLine(int x1, int y1, int x2, int y2)
{
	drawSubLine(x1, y1, x2, y2);
}

void CanvasXtrme::drawLines(const Point* pnts, int npnts)
{
	for (int i = 0; i < npnts - 1; ++i)
		drawLine(pnts[i].x, pnts[i].y, pnts[i + 1].x, pnts[i + 1].y);
}

void CanvasXtrme::fillCircle(int x, int y, float radius)
{
	m_batchRenderer.batch(BatchRenderer::PiDefault, 0, render::PtTriangles);

	for (int i = 0, j = c_circleResolution - 1; i < c_circleResolution; j = i++)
	{
		float a1 = (float(i) / (c_circleResolution - 1)) * PI * 2.0f;
		float a2 = (float(j) / (c_circleResolution - 1)) * PI * 2.0f;

		int x1 = int(cos(a1) * radius + x);
		int y1 = int(sin(a1) * radius + y);

		int x2 = int(cos(a2) * radius + x);
		int y2 = int(sin(a2) * radius + y);

		m_batchRenderer.add(x, y, m_backGround);
		m_batchRenderer.add(x1, y1, m_backGround);
		m_batchRenderer.add(x2, y2, m_backGround);
	}
}

void CanvasXtrme::drawCircle(int x, int y, float radius)
{
	for (int i = 0, j = c_circleResolution - 1; i < c_circleResolution; j = i++)
	{
		float a1 = (float(i) / (c_circleResolution - 1)) * PI * 2.0f;
		float a2 = (float(j) / (c_circleResolution - 1)) * PI * 2.0f;

		int x1 = int(cos(a1) * radius + x);
		int y1 = int(sin(a1) * radius + y);

		int x2 = int(cos(a2) * radius + x);
		int y2 = int(sin(a2) * radius + y);

		drawSubLine(x1, y1, x2, y2);
	}
}

void CanvasXtrme::drawEllipticArc(int x, int y, int w, int h, float start, float end)
{
	for (int i = 0; i < c_circleResolution - 1; ++i)
	{
		float a1 = start + (float(i    ) / (c_circleResolution - 1)) * (end - start);
		float a2 = start + (float(i + 1) / (c_circleResolution - 1)) * (end - start);

		a1 *= PI / 180.0f;
		a2 *= PI / 180.0f;

		int x1 = int(cos(a1) * w / 2.0f + x + w / 2.0f);
		int y1 = int(sin(a1) * h / 2.0f + y + h / 2.0f);

		int x2 = int(cos(a2) * w / 2.0f + x + w / 2.0f);
		int y2 = int(sin(a2) * h / 2.0f + y + h / 2.0f);

		drawSubLine(x1, y1, x2, y2);
	}
}

namespace
{

	struct KeyPoint
	{
		float T;
		Vector2 value;
	};

}

void CanvasXtrme::drawSpline(const Point* pnts, int npnts)
{
	if (npnts <= 1)
		return;

	std::vector< KeyPoint > kp(npnts);
	for (int i = 0; i < npnts; ++i)
	{
		kp[i].T = float(i) / (npnts - 1);
		kp[i].value.x = float(pnts[i].x);
		kp[i].value.y = float(pnts[i].y);
	}

	float t = 0.0f;
	float s = 0.2f;
	while (t < 1.0f)
	{
		Vector2 ps = Hermite< KeyPoint, float, Vector2 >::evaluate(&kp[0], kp.size(), t);
		Vector2 pm = Hermite< KeyPoint, float, Vector2 >::evaluate(&kp[0], kp.size(), t + s / 2.0f);
		Vector2 pe = Hermite< KeyPoint, float, Vector2 >::evaluate(&kp[0], kp.size(), t + s);

		float err = (pm - (ps + pe) / 2.0f).length();
		if (err < 0.2f || s < 0.01f)
		{
			drawLine(
				int(ps.x), int(ps.y),
				int(pe.x), int(pe.y)
			);
			t += s; s = 0.2f;
		}
		else
			s /= 2.0f;
	}
}

void CanvasXtrme::fillRect(const Rect& rc)
{
	m_batchRenderer.batch(BatchRenderer::PiDefault, 0, render::PtTriangleStrip);
	m_batchRenderer.add(rc.left, rc.top, m_backGround);
	m_batchRenderer.add(rc.right, rc.top, m_backGround);
	m_batchRenderer.add(rc.left, rc.bottom, m_backGround);
	m_batchRenderer.add(rc.right, rc.bottom, m_backGround);
}

void CanvasXtrme::fillGradientRect(const Rect& rc, bool vertical)
{
	m_batchRenderer.batch(BatchRenderer::PiDefault, 0, render::PtTriangleStrip);
	m_batchRenderer.add(rc.left, rc.top, m_foreGround);
	m_batchRenderer.add(rc.right, rc.top, m_foreGround);
	m_batchRenderer.add(rc.left, rc.bottom, m_backGround);
	m_batchRenderer.add(rc.right, rc.bottom, m_backGround);
}

void CanvasXtrme::drawRect(const Rect& rc)
{
	drawLine(rc.left, rc.top, rc.right, rc.top);
	drawLine(rc.right, rc.top, rc.right, rc.bottom);
	drawLine(rc.right, rc.bottom, rc.left, rc.bottom);
	drawLine(rc.left, rc.bottom, rc.left, rc.top);
}

void CanvasXtrme::drawRoundRect(const Rect& rc, int radius)
{
	drawLine(rc.left, rc.top, rc.right, rc.top);
	drawLine(rc.right, rc.top, rc.right, rc.bottom);
	drawLine(rc.right, rc.bottom, rc.left, rc.bottom);
	drawLine(rc.left, rc.bottom, rc.left, rc.top);
}

void CanvasXtrme::drawPolygon(const Point* pnts, int count)
{
	for (int i = 0; i < count - 1; ++i)
		drawLine(pnts[i].x, pnts[i].y, pnts[i + 1].x, pnts[i + 1].y);
	drawLine(pnts[count - 1].x, pnts[count - 1].y, pnts[0].x, pnts[0].y);
}

void CanvasXtrme::fillPolygon(const Point* pnts, int count)
{
	m_batchRenderer.batch(BatchRenderer::PiDefault, 0, render::PtTriangles);
	for (int i = 0; i < count - 2; ++i)
	{
		m_batchRenderer.add(pnts[0].x, pnts[0].y, m_backGround);
		m_batchRenderer.add(pnts[i + 1].x, pnts[i + 1].y, m_backGround);
		m_batchRenderer.add(pnts[i + 2].x, pnts[i + 2].y, m_backGround);
	}
}

void CanvasXtrme::drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, IBitmap* bitmap, uint32_t blendMode)
{
	drawBitmap(dstAt, size, srcAt, size, bitmap, blendMode);
}

void CanvasXtrme::drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, uint32_t blendMode)
{
	Ref< render::ITexture > texture = m_imageCache.getTexture(bitmap);
	T_ASSERT (texture);
	
	Size size = bitmap->getSize();

	float u1 = float(srcAt.x) / size.cx;
	float v1 = float(srcAt.y) / size.cy;
	float u2 = u1 + float(srcSize.cx) / size.cx;
	float v2 = v1 + float(srcSize.cy) / size.cy;

	uint32_t modulate = 0xffffffff;
	if ((blendMode & BmModulate) != 0)
		modulate = m_backGround;

	m_batchRenderer.batch(BatchRenderer::PiImage, texture, render::PtTriangleStrip);
	m_batchRenderer.add(dstAt.x, dstAt.y, u1, v1, modulate);
	m_batchRenderer.add(dstAt.x + dstSize.cx, dstAt.y, u2, v1, modulate);
	m_batchRenderer.add(dstAt.x, dstAt.y + dstSize.cy, u1, v2, modulate);
	m_batchRenderer.add(dstAt.x + dstSize.cx, dstAt.y + dstSize.cy, u2, v2, modulate);
}

void CanvasXtrme::drawText(const Point& at, const std::wstring& text)
{
	if (text.empty())
		return;

	m_batchRenderer.batch(BatchRenderer::PiFont, m_currentFontMap->getTexture(), render::PtTriangles);

	int x1 = at.x;
	int y1 = at.y;

	int kerning = 0;

	for (size_t i = 0; i < text.length(); ++i)
	{
		const FontMap::Glyph& glyph = m_currentFontMap->getGlyph(text[i]);

		int x2 = x1 + glyph.size[0];
		int y2 = y1 + glyph.size[1];

		kerning += i > 0 ? m_currentFontMap->getKerning(text[i - 1], text[i]) : 0;

		m_batchRenderer.add(x1 + kerning, y1, glyph.uv[0], glyph.uv[1], 0xffffffff);
		m_batchRenderer.add(x2 + kerning, y1, glyph.uv[2], glyph.uv[1], 0xffffffff);
		m_batchRenderer.add(x1 + kerning, y2, glyph.uv[0], glyph.uv[3], 0xffffffff);
		m_batchRenderer.add(x2 + kerning, y1, glyph.uv[2], glyph.uv[1], 0xffffffff);
		m_batchRenderer.add(x2 + kerning, y2, glyph.uv[2], glyph.uv[3], 0xffffffff);
		m_batchRenderer.add(x1 + kerning, y2, glyph.uv[0], glyph.uv[3], 0xffffffff);

		x1 = x2;
	}
}

void CanvasXtrme::drawText(const Rect& rc, const std::wstring& text, Align halign, Align valign)
{
	Size extent = getTextExtent(text);

	int x = rc.left;
	int y = rc.top;

	switch (halign)
	{
	case AnLeft:
		break;

	case AnCenter:
		x += (rc.getWidth() - extent.cx) / 2;
		break;

	case AnRight:
		x = rc.getWidth() - extent.cx;
		break;
	}

	switch (valign)
	{
	case AnTop:
		break;

	case AnCenter:
		y += (rc.getHeight() - extent.cy) / 2;
		break;

	case AnBottom:
		y = rc.getHeight() - extent.cy;
		break;
	}

	drawText(Point(x, y), text);
}

Size CanvasXtrme::getTextExtent(const std::wstring& text) const
{
	T_ASSERT_M (m_currentFontMap, L"No font selected");
	return m_currentFontMap->getExtent(text);
}

void CanvasXtrme::begin()
{
	m_batchRenderer.begin();
}

void CanvasXtrme::end(const Size& size)
{
	m_batchRenderer.end(size);
	m_imageCache.flush();
}

void CanvasXtrme::drawSubLine(int x1, int y1, int x2, int y2)
{
	if (m_thickness <= 1)
	{
		m_batchRenderer.batch(BatchRenderer::PiDefault, 0, render::PtLines);
		m_batchRenderer.add(x1, y1, m_foreGround);
		m_batchRenderer.add(x2, y2, m_foreGround);
	}
	else
	{
		float dx = float(x2 - x1);
		float dy = float(y2 - y1);

		float ln = sqrt(dx * dx + dy * dy) * 2.0f / m_thickness;
		dx /= ln;
		dy /= ln;

		float px = dy;
		float py = -dx;

		float fx1 = x1 - dx;
		float fy1 = y1 - dy;

		float fx2 = x2 + dx;
		float fy2 = y2 + dy;

		m_batchRenderer.batch(BatchRenderer::PiDefault, 0, render::PtTriangleStrip);
		m_batchRenderer.add(int(fx1 - px), int(fy1 - py), m_foreGround);
		m_batchRenderer.add(int(fx1 + px), int(fy1 + py), m_foreGround);
		m_batchRenderer.add(int(fx2 - px), int(fy2 - py), m_foreGround);
		m_batchRenderer.add(int(fx2 + px), int(fy2 + py), m_foreGround);
	}
}

		}
	}
}
