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
	explicit UserWidgetX11(Context* context, EventSubject* owner);

	virtual bool create(IWidget* parent, int style) override final;
};

	}
}

#endif	// traktor_ui_UserWidgetX11_H

