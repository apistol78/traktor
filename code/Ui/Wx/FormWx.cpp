#include <wx/wx.h>
#include "Core/Misc/AutoPtr.h"
#include "Ui/Events/CloseEvent.h"
#include "Ui/Wx/FormWx.h"
#include "Ui/Wx/MenuBarWx.h"

namespace traktor
{
	namespace ui
	{

FormWx::FormWx(EventSubject* owner)
:	WidgetWxImpl< IForm, wxFrame >(owner)
,	m_unbufferedPaint(false)
,	m_toolBar(0)
{
}

bool FormWx::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	int wxStyle = wxCLIP_SIBLINGS | wxCLIP_CHILDREN | wxFULL_REPAINT_ON_RESIZE;
	if (style & WsBorder)
		wxStyle |= wxSIMPLE_BORDER;
	if (style & WsClientBorder)
		wxStyle |= wxSUNKEN_BORDER;
	if (style & WsResizable)
		wxStyle |= wxRESIZE_BORDER;
	if (style & WsSystemBox)
		wxStyle |= wxSYSTEM_MENU;
	if (style & WsMinimizeBox)
		wxStyle |= wxMINIMIZE_BOX;
	if (style & WsMaximizeBox)
		wxStyle |= wxMAXIMIZE_BOX;
	if (style & WsCloseBox)
		wxStyle |= wxCLOSE_BOX;
	if (style & WsCaption)
		wxStyle |= wxCAPTION;
	if (style & WsTop)
		wxStyle |= wxSTAY_ON_TOP;

	m_window = new wxFrame();
	
	if (!m_window->Create(
		parent ? static_cast< wxWindow* >(parent->getInternalHandle()) : 0,
		-1,
		wstots(text).c_str(),
		wxDefaultPosition,
		wxSize(width, height),
		wxStyle
	))
	{
		m_window->Destroy();
		return false;
	}

#if defined(__WXMSW__)
	m_window->SetIcon(wxIcon(_T("DEFAULTICON"), wxBITMAP_TYPE_ICO_RESOURCE));
	m_window->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
#endif

	if (!WidgetWxImpl< IForm, wxFrame >::create(style))
		return false;

	T_CONNECT(m_window, wxEVT_CLOSE_WINDOW, wxCloseEvent, FormWx, &FormWx::onClose);
	
	return true;
}

void FormWx::setIcon(ISystemBitmap* icon)
{
}

void FormWx::maximize()
{
	m_window->Maximize(true);
}

void FormWx::minimize()
{
	m_window->Iconize();
}

void FormWx::restore()
{
	m_window->Maximize(false);
}

bool FormWx::isMaximized() const
{
	return m_window->IsMaximized();
}

bool FormWx::isMinimized() const
{
	return m_window->IsIconized();
}

void FormWx::hideProgress()
{
}

void FormWx::showProgress(int32_t current, int32_t total)
{
}

Rect FormWx::getInnerRect() const
{
	Rect inner = WidgetWxImpl< IForm, wxFrame >::getInnerRect();
#if defined(__WXMSW__) && defined(T_USE_COOLBAR)
	if (m_coolBar)
	{
		int coolBarHeight = m_coolBar->GetSize().y;
		inner.top += coolBarHeight;
	}
#else
	if (m_toolBar)
	{
		int toolBarHeight = m_toolBar->GetSize().y;
		inner.top += toolBarHeight;
	}
#endif
	return inner;
}

void FormWx::setOutline(const Point* p, int np)
{
	if (p && np > 0)
	{
		AutoArrayPtr< wxPoint > points(new wxPoint [np]);
		for (int i = 0; i < np; ++i)
			points[i] = wxPoint(p[i].x, p[i].y);

		wxRegion region(size_t(np), points.ptr());
		m_window->SetShape(region);
	}
	else
		m_window->SetShape(wxRegion());
}

void FormWx::update(const Rect* rc, bool immediate)
{
	Rect inner = WidgetWxImpl< IForm, wxFrame >::getInnerRect();
#if defined(__WXMSW__) && defined(T_USE_COOLBAR)
	if (m_coolBar)
	{
		int coolBarHeight = m_coolBar->GetSize().y;
		m_coolBar->SetSize(inner.getWidth(), coolBarHeight);
	}
#else
	if (m_toolBar)
	{
		int toolBarHeight = m_toolBar->GetSize().y;
		m_toolBar->SetSize(inner.getWidth(), toolBarHeight);
	}
#endif
	WidgetWxImpl< IForm, wxFrame >::update(rc, immediate);
}

void FormWx::onClose(wxCloseEvent& event)
{
	CloseEvent closeEvent(m_owner);
	m_owner->raiseEvent(&closeEvent);
	if (closeEvent.consumed() && closeEvent.cancelled())
		event.Veto();
}

	}
}
