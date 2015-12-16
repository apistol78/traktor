#include <wx/wx.h>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/Wx/BitmapWx.h"
#include "Ui/Wx/CanvasWx.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

wxColour toWxColour(const Color4ub& c)
{
	return wxColour(
		c.r,
		c.g,
		c.b,
		c.a
	);
}

		}

bool CanvasWx::beginPaint(wxWindow* window, bool doubleBuffer)
{
	m_font = window->GetFont();
	m_foreGround = *wxBLACK;
	m_backGround = *wxWHITE;

	m_context = wxGraphicsRenderer::GetDefaultRenderer()->CreateContext(window);
	m_context->SetFont(m_font, m_foreGround);
	return true;
}

void CanvasWx::endPaint(wxWindow* window)
{
	if (m_context)
	{
		delete m_context;
		m_context = 0;
	}
}

void CanvasWx::setForeground(const Color4ub& color)
{
	m_foreGround = toWxColour(color);
}

void CanvasWx::setBackground(const Color4ub& color)
{
	m_backGround = toWxColour(color);
}

void CanvasWx::setFont(const Font& font)
{
	m_font = wxFont(
#if defined(__WXGTK__)
		font.getPixelSize() - 6,
#else
		font.getPixelSize(),
#endif
		wxFONTFAMILY_DEFAULT,
		font.isItalic() ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
		font.isBold() ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL,
		font.isUnderline(),
		wstots(font.getFace()).c_str(),
		wxFONTENCODING_DEFAULT
	);
}

void CanvasWx::setLineStyle(LineStyle lineStyle)
{
	/*
	wxPen pen = m_dc->GetPen();
	switch (lineStyle)
	{
	case LsSolid:
		pen.SetStyle(wxSOLID);
		break;

	case LsDot:
		pen.SetStyle(wxDOT);
		break;

	case LsDotDash:
		pen.SetStyle(wxDOT_DASH);
		break;
	}
	m_dc->SetPen(pen);
	*/
}

void CanvasWx::setPenThickness(int thickness)
{
	/*
	wxPen pen = m_dc->GetPen();
	pen.SetWidth(thickness);
	m_dc->SetPen(pen);
	*/
}

void CanvasWx::setClipRect(const Rect& rc)
{
	m_context->Clip(wxRegion(
		rc.left,
		rc.top,
		rc.getWidth(),
		rc.getHeight()
	));
}

void CanvasWx::resetClipRect()
{
	m_context->ResetClip();
}

void CanvasWx::drawPixel(int x, int y, const Color4ub& c)
{
	/*
	wxPen currentPen = m_dc->GetPen();
	m_dc->SetPen(wxPen(toWxColour(c), 1));
	m_dc->DrawPoint(x, y);
	m_dc->SetPen(currentPen);
	*/
}

void CanvasWx::drawLine(int x1, int y1, int x2, int y2)
{
	m_context->SetPen(wxPen(m_foreGround));
	m_context->StrokeLine(x1, y1, x2, y2);
}

void CanvasWx::drawLines(const Point* pnts, int npnts)
{
	m_context->SetPen(wxPen(m_foreGround));
	for (int i = 0; i < npnts - 1; ++i)
	{
		m_context->StrokeLine(
			pnts[i].x, pnts[i].y,
			pnts[i + 1].x, pnts[i + 1].y
		);
	}
}

void CanvasWx::fillCircle(int x, int y, float radius)
{
	m_context->SetPen(wxPen(*wxBLACK, 0, wxTRANSPARENT));
	m_context->SetBrush(wxBrush(m_backGround));
	m_context->DrawEllipse(x, y, radius, radius);
}

void CanvasWx::drawCircle(int x, int y, float radius)
{
	m_context->SetPen(wxPen(m_foreGround));
	m_context->SetBrush(wxBrush(*wxBLACK, wxTRANSPARENT));
	m_context->DrawEllipse(x, y, radius, radius);
}

void CanvasWx::drawEllipticArc(int x, int y, int w, int h, float start, float end)
{
	/*
	wxBrush tmp = m_dc->GetBrush();
	m_dc->SetBrush(wxBrush(*wxBLACK, wxTRANSPARENT));
	m_dc->DrawEllipticArc(x, y, w, h, start, end);
	m_dc->SetBrush(tmp);
	*/
}

void CanvasWx::drawSpline(const Point* pnts, int npnts)
{
	/*
	std::vector< wxPoint > wxp(npnts);
	for (int i = 0; i < npnts; ++i)
	{
		wxp[i].x = pnts[i].x;
		wxp[i].y = pnts[i].y;
	}
	m_dc->DrawSpline(npnts, &wxp[0]);
	*/
}

void CanvasWx::fillRect(const Rect& rc)
{
	m_context->SetPen(wxPen(*wxBLACK, 0, wxTRANSPARENT));
	m_context->SetBrush(wxBrush(m_backGround));
	m_context->DrawRectangle(rc.left, rc.top, rc.getWidth(), rc.getHeight());
}

void CanvasWx::fillGradientRect(const Rect& rc, bool vertical)
{
	wxGraphicsBrush b;

	if (vertical)
		b = m_context->CreateLinearGradientBrush(
			rc.left, rc.top,
			rc.left, rc.bottom,
			m_foreGround,
			m_backGround
		);
	else
		b = m_context->CreateLinearGradientBrush(
			rc.left, rc.top,
			rc.right, rc.top,
			m_foreGround,
			m_backGround
		);

	m_context->SetPen(wxPen(*wxBLACK, 0, wxTRANSPARENT));
	m_context->SetBrush(b);
	m_context->DrawRectangle(rc.left, rc.top, rc.getWidth(), rc.getHeight());
}

void CanvasWx::drawRect(const Rect& rc)
{
	m_context->SetPen(wxPen(m_foreGround));
	m_context->SetBrush(wxBrush(*wxBLACK, wxTRANSPARENT));
	m_context->DrawRectangle(rc.left, rc.top, rc.getWidth(), rc.getHeight());
}

void CanvasWx::drawRoundRect(const Rect& rc, int radius)
{
	m_context->SetPen(wxPen(m_foreGround));
	m_context->SetBrush(wxBrush(*wxBLACK, wxTRANSPARENT));
	m_context->DrawRoundedRectangle(rc.left, rc.top, rc.getWidth(), rc.getHeight(), radius);
}

void CanvasWx::drawPolygon(const Point* pnts, int count)
{
	/*
	std::vector< wxPoint > points(count);
	for (int i = 0; i < count; ++i)
	{
		points[i].x = pnts[i].x;
		points[i].y = pnts[i].y;
	}
	wxBrush tmp = m_dc->GetBrush();
	m_dc->SetBrush(wxBrush(*wxBLACK, wxTRANSPARENT));
	m_dc->DrawPolygon(count, &points[0]);
	m_dc->SetBrush(tmp);
	*/
}

void CanvasWx::fillPolygon(const Point* pnts, int count)
{
	/*
	std::vector< wxPoint > points(count);
	for (int i = 0; i < count; ++i)
	{
		points[i].x = pnts[i].x;
		points[i].y = pnts[i].y;
	}
	wxPen tmp = m_dc->GetPen();
	m_dc->SetPen(wxPen(*wxBLACK, 0, wxTRANSPARENT));
	m_dc->DrawPolygon(count, &points[0]);
	m_dc->SetPen(tmp);
	*/
}

void CanvasWx::drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, IBitmap* bitmap, uint32_t blendMode)
{
	if (!bitmap)
		return;

	wxImage* image = reinterpret_cast< BitmapWx* >(bitmap)->getWxImage();
	if (!image)
		return;

	m_context->DrawBitmap(
		image->GetSubImage(wxRect(srcAt.x, srcAt.y, size.cx, size.cy)),
		dstAt.x, dstAt.y,
		size.cx, size.cy
	);
}

void CanvasWx::drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, uint32_t blendMode)
{
	if (!bitmap)
		return;

	wxImage* image = reinterpret_cast< BitmapWx* >(bitmap)->getWxImage();
	if (!image)
		return;

	m_context->DrawBitmap(
		image->GetSubImage(wxRect(srcAt.x, srcAt.y, srcSize.cx, srcSize.cy)),
		dstAt.x, dstAt.y,
		dstSize.cx, dstSize.cy
	);
}

void CanvasWx::drawText(const Point& at, const std::wstring& text)
{
	if (text.empty())
		return;

	tstring tmp = wstots(text);
	m_context->SetFont(m_font, m_foreGround);
	m_context->DrawText(tmp.c_str(), at.x, at.y);
}

void CanvasWx::drawText(const Rect& rc, const std::wstring& text, Align halign, Align valign)
{
	if (text.empty())
		return;

	tstring tmp = wstots(text);
	wxPoint pt(rc.left, rc.top);

	wxDouble w, h;
	m_context->SetFont(m_font, m_foreGround);
	m_context->GetTextExtent(tmp.c_str(), &w, &h);

	switch (halign)
	{
	case AnLeft:
		break;

	case AnCenter:
		pt.x = rc.left + (rc.getWidth() - w) / 2;
		break;

	case AnRight:
		pt.x = rc.right - w;
		break;
	}

	switch (valign)
	{
	case AnTop:
		break;

	case AnCenter:
		pt.y = rc.top + (rc.getHeight() - h) / 2;
		break;

	case AnBottom:
		pt.y = rc.bottom - h;
		break;
	}

	m_context->DrawText(tmp.c_str(), pt.x, pt.y);
}

Size CanvasWx::getTextExtent(const std::wstring& text) const
{
	wxDouble w, h;
	tstring tmp = wstots(text);
	m_context->SetFont(m_font, m_foreGround);
	m_context->GetTextExtent(tmp.c_str(), &w, &h);
	return Size(w, h);
}

void* CanvasWx::getSystemHandle()
{
	return 0;
}

	}
}
