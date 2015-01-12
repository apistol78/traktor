#include "Ui/Wx/ToolFormWx.h"
#include "Ui/Events/CloseEvent.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{
	namespace ui
	{

ToolFormWx::ToolFormWx(EventSubject* owner)
:	WidgetWxImpl< IToolForm, wxMiniFrame >(owner)
{
}

bool ToolFormWx::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	int wxStyle = wxCLIP_CHILDREN | wxFRAME_SHAPED;
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
	if ((style & (WsBorder | WsResizable)) == 0)
		wxStyle |= wxNO_BORDER;

	m_window = new wxMiniFrame();

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

	if (!WidgetWxImpl< IToolForm, wxMiniFrame >::create(style))
		return false;

	T_CONNECT(m_window, wxEVT_CLOSE_WINDOW, wxCloseEvent, ToolFormWx, &ToolFormWx::onClose);

	return true;
}

void ToolFormWx::center()
{
	m_window->CentreOnScreen();
}

void ToolFormWx::setOutline(const Point* p, int np)
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

void ToolFormWx::onClose(wxCloseEvent& event)
{
	CloseEvent closeEvent(m_owner);
	m_owner->raiseEvent(&closeEvent);
}

	}
}
