#ifndef traktor_ui_StaticWin32_H
#define traktor_ui_StaticWin32_H

#include "Ui/Win32/WidgetWin32Impl.h"
#include "Ui/Itf/IStatic.h"

namespace traktor
{
	namespace ui
	{

class StaticWin32 : public WidgetWin32Impl< IStatic >
{
public:
	StaticWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text);

	virtual Size getPreferedSize() const;
};

	}
}

#endif	// traktor_ui_StaticWin32_H
