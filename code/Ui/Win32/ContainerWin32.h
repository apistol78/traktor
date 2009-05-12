#ifndef traktor_ui_ContainerWin32_H
#define traktor_ui_ContainerWin32_H

#include "Ui/Win32/WidgetWin32Impl.h"
#include "Ui/Itf/IContainer.h"

namespace traktor
{
	namespace ui
	{

class ContainerWin32 : public WidgetWin32Impl< IContainer >
{
public:
	ContainerWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);
};

	}
}

#endif	// traktor_ui_ContainerWin32_H
