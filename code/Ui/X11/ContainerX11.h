#ifndef traktor_ui_ContainerX11_H
#define traktor_ui_ContainerX11_H

#include "Ui/X11/WidgetX11Impl.h"
#include "Ui/Itf/IContainer.h"

namespace traktor
{
	namespace ui
	{

class ContainerX11 : public WidgetX11Impl< IContainer >
{
public:
	ContainerX11(EventSubject* owner, Display* display, int32_t screen, XIM xim);

	virtual bool create(IWidget* parent, int style) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_ui_ContainerX11_H
