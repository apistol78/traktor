#ifndef traktor_ui_UserWidgetX11_H
#define traktor_ui_UserWidgetX11_H

#include "Ui/X11/WidgetX11Impl.h"
#include "Ui/Itf/IUserWidget.h"

namespace traktor
{
	namespace ui
	{

class UserWidgetX11 : public WidgetX11Impl< IUserWidget >
{
public:
	UserWidgetX11(EventSubject* owner, Display* display, int32_t screen);

	virtual bool create(IWidget* parent, int style) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_ui_UserWidgetX11_H

