#include "Ui/Wx/UserWidgetWx.h"

namespace traktor
{
	namespace ui
	{

UserWidgetWx::UserWidgetWx(EventSubject* owner)
:	WidgetWxImpl< IUserWidget, wxWindow >(owner)
{
}

bool UserWidgetWx::create(IWidget* parent, int style)
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

	m_window = new wxWindow(
		parent ? static_cast< wxWindow* >(parent->getInternalHandle()) : 0,
		-1,
		wxDefaultPosition,
		wxDefaultSize,
		wxStyle
	);

	if (!WidgetWxImpl< IUserWidget, wxWindow >::create(style))
		return false;

	return true;
}

	}
}
