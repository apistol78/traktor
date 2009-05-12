#ifndef traktor_ui_NativeWin32_H
#define traktor_ui_NativeWin32_H

#include "Ui/Win32/WidgetWin32Impl.h"
#include "Ui/Itf/INative.h"

namespace traktor
{
	namespace ui
	{

class NativeWin32 : public WidgetWin32Impl< INative >
{
public:
	NativeWin32(EventSubject* owner);

	virtual bool create(void* nativeHandle);
};

	}
}

#endif	// traktor_ui_NativeWin32_H
