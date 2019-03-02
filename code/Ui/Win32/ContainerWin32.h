#pragma once

#include "Ui/Itf/IContainer.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class ContainerWin32 : public WidgetWin32Impl< IContainer >
{
public:
	ContainerWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);
};

	}
}

