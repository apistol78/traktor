#include <map>
#include <wx/wx.h>
#include <wx/dcbuffer.h>
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
		c.b
	);
}

		}

bool CanvasWx::beginPaint(wxWindow* window, bool doubleBuffer)
{
	m_dc = new wxPaintDC(window);
#if wxUSE_GRAPHICS_CONTEXT
	m_context = wxGraphicsContext::Create(*m_dc);
#endif

	m_dc->SetFont(window->GetFont());
	m_dc->SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));

	return true;
}

void CanvasWx::endPaint(wxWindow* window)
{
#if wxUSE_GRAPHICS_CONTEXT
	if (m_context)
	{
		delete m_context;
		m_context = 0;
	}
#endif
	if (m_dc)
	{
		m_dc->SetPen(wxNullPen);
		delete m_dc;
		m_dc = 0;
	}
}

void CanvasWx::setForeground(const Color4ub& color)
{
	m_foreGround = color;
	m_dc->SetPen(wxPen(toWxColour(m_foreGround), 1));
	m_dc->SetTextForeground(toWxColour(m_foreGround));
}

void CanvasWx::setBackground(const Color4ub& color)
{
	m_backGround = color;
	m_dc->SetBrush(wxBrush(toWxColour(m_backGround)));
}

void CanvasWx::setFont(const Font& font)
{
	m_dc->SetFont(wxFont(
		font.getSize(),
		wxFONTFAMILY_DEFAULT,
		font.isItalic() ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
		font.isBold() ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL,
		font.isUnderline(),
		wstots(font.getFace()).c_str(),
		wxFONTENCODING_DEFAULT
	));
}

void CanvasWx::setLineStyle(LineStyle lineStyle)
{
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
}

void CanvasWx::setPenThickness(int thickness)
{
	wxPen pen = m_dc->GetPen();
	pen.SetWidth(thickness);
	m_dc->SetPen(pen);
}

void CanvasWx::setClipRect(const Rect& rc)
{
	m_dc->DestroyClippingRegion();
	m_dc->SetClippingRegion(rc.left, rc.top, rc.getWidth(), rc.getHeight());
}

void CanvasWx::resetClipRect()
{
	m_dc->DestroyClippingRegion();
}

void CanvasWx::drawPixel(int x, int y, const Color4ub& c)
{
	wxPen currentPen = m_dc->GetPen();
	m_dc->SetPen(wxPen(toWxColour(c), 1));
	m_dc->DrawPoint(x, y);
	m_dc->SetPen(currentPen);
}

void CanvasWx::drawLine(int x1, int y1, int x2, int y2)
{
	m_dc->DrawLine(x1, y1, x2, y2);
	m_dc->DrawPoint(x2, y2);
}

void CanvasWx::drawLines(const Point* pnts, int npnts)
{
	std::vector< wxPoint > wxp(npnts);
	for (int i = 0; i < npnts; ++i)
	{
		wxp[i].x = pnts[i].x;
		wxp[i].y = pnts[i].y;
	}
	m_dc->DrawLines(npnts, &wxp[0]);
}

void CanvasWx::fillCircle(int x, int y, float radius)
{
	wxPen tmp = m_dc->GetPen();
	m_dc->SetPen(wxPen(*wxBLACK, 0, wxTRANSPARENT));
	m_dc->DrawCircle(x, y, wxCoord(radius));
	m_dc->SetPen(tmp);
}

void CanvasWx::drawCircle(int x, int y, float radius)
{
	wxBrush tmp = m_dc->GetBrush();
	m_dc->SetBrush(wxBrush(*wxBLACK, wxTRANSPARENT));
	m_dc->DrawCircle(x, y, wxCoord(radius));
	m_dc->SetBrush(tmp);
}

void CanvasWx::drawEllipticArc(int x, int y, int w, int h, float start, float end)
{
	wxBrush tmp = m_dc->GetBrush();
	m_dc->SetBrush(wxBrush(*wxBLACK, wxTRANSPARENT));
	m_dc->DrawEllipticArc(x, y, w, h, start, end);
	m_dc->SetBrush(tmp);
}

void CanvasWx::drawSpline(const Point* pnts, int npnts)
{
	std::vector< wxPoint > wxp(npnts);
	for (int i = 0; i < npnts; ++i)
	{
		wxp[i].x = pnts[i].x;
		wxp[i].y = pnts[i].y;
	}
	m_dc->DrawSpline(npnts, &wxp[0]);
}

void CanvasWx::fillRect(const Rect& rc)
{
	wxPen tmp = m_dc->GetPen();
	m_dc->SetPen(wxPen(*wxBLACK, 0, wxTRANSPARENT));
	m_dc->DrawRectangle(rc.left, rc.top, rc.getWidth(), rc.getHeight());
	m_dc->SetPen(tmp);
}

void CanvasWx::fillGradientRect(const Rect& rc, bool vertical)
{
	wxColour from = toWxColour(m_foreGround);
	wxColour to = toWxColour(m_backGround);
	m_dc->GradientFillLinear(
		wxRect(rc.left, rc.top, rc.getWidth() + 1, rc.getHeight() + 1),
		from,
		to,
		vertical ? wxSOUTH : wxEAST
	);
}

void CanvasWx::drawRect(const Rect& rc)
{
	wxBrush tmp = m_dc->GetBrush();
	m_dc->SetBrush(wxBrush(*wxBLACK, wxTRANSPARENT));
	m_dc->DrawRectangle(rc.left, rc.top, rc.getWidth(), rc.getHeight());
	m_dc->SetBrush(tmp);
}

void CanvasWx::drawRoundRect(const Rect& rc, int radius)
{
	wxBrush tmp = m_dc->GetBrush();
	m_dc->SetBrush(wxBrush(*wxBLACK, wxTRANSPARENT));
	m_dc->DrawRoundedRectangle(rc.left, rc.top, rc.getWidth(), rc.getHeight(), radius);
	m_dc->SetBrush(tmp);
}

void CanvasWx::drawPolygon(const Point* pnts, int count)
{
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
}

void CanvasWx::fillPolygon(const Point* pnts, int count)
{
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
}

void CanvasWx::drawBitmap(const Point& dstAt, const Point& srcAt, const Size& size, IBitmap* bitmap, uint32_t blendMode)
{
	if (!bitmap)
		return;

	wxImage* image = reinterpret_cast< BitmapWx* >(bitmap)->getWxImage();
	if (!image)
		return;

	wxDCClipper clipper(*m_dc, dstAt.x, dstAt.y, size.cx, size.cy);
	m_dc->DrawBitmap(
		image->GetSubImage(wxRect(srcAt.x, srcAt.y, size.cx, size.cy)),
		dstAt.x, dstAt.y,
		blendMode == BmAlpha
	);
}

void CanvasWx::drawBitmap(const Point& dstAt, const Size& dstSize, const Point& srcAt, const Size& srcSize, IBitmap* bitmap, uint32_t blendMode)
{
	if (!bitmap)
		return;

	wxImage* image = reinterpret_cast< BitmapWx* >(bitmap)->getWxImage();
	if (!image)
		return;

#if wxUSE_GRAPHICS_CONTEXT
	m_context->DrawBitmap(
		image->GetSubImage(wxRect(srcAt.x, srcAt.y, srcSize.cx, srcSize.cy)),
		dstAt.x, dstAt.y,
		dstSize.cx, dstSize.cy
	);
#endif
}

void CanvasWx::drawText(const Point& at, const std::wstring& text)
{
	if (text.empty())
		return;

	if (m_dc->GetFont().IsNull() || !m_dc->GetFont().IsOk())
		return;

	tstring tmp = wstots(text);
	m_dc->DrawText(tmp.c_str(), at.x, at.y);
}

void CanvasWx::drawText(const Rect& rc, const std::wstring& text, Align halign, Align valign)
{
	if (text.empty())
		return;

	if (m_dc->GetFont().IsNull() || !m_dc->GetFont().IsOk())
		return;

	tstring tmp = wstots(text);
	wxPoint pt(rc.left, rc.top);

	int w, h;
	m_dc->GetTextExtent(tmp.c_str(), &w, &h);

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

	m_dc->DrawText(tmp.c_str(), pt.x, pt.y);
}

Size CanvasWx::getTextExtent(const std::wstring& text) const
{
	wxCoord w, h;
	tstring tmp = wstots(text);
	m_dc->GetTextExtent(tmp.c_str(), &w, &h);
	return Size(w, h);
}

void* CanvasWx::getSystemHandle()
{
	return 0;
}

	}
}
