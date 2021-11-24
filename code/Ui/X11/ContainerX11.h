#pragma once

#include "Ui/X11/WidgetX11Impl.h"
#include "Ui/Itf/IContainer.h"

namespace traktor
{
	namespace ui
	{

class ContainerX11 : public WidgetX11Impl< IContainer >
{
public:
	ContainerX11(Context* context, EventSubject* owner);

	virtual bool create(IWidget* parent, int style) override final;
};

	}
}

