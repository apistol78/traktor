#ifndef traktor_ui_ToolFormWx_H
#define traktor_ui_ToolFormWx_H

#include <wx/wx.h>
#include <wx/minifram.h>
#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IToolForm.h"

namespace traktor
{
	namespace ui
	{

class ToolFormWx : public WidgetWxImpl< IToolForm, wxMiniFrame >
{
public:
	ToolFormWx(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style);

	virtual void center();

private:
	void onClose(wxCloseEvent& event);
};

	}
}

#endif	// traktor_ui_ToolFormWx_H
