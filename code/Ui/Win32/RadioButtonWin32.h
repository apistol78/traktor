#ifndef traktor_ui_RadioButtonWin32_H
#define traktor_ui_RadioButtonWin32_H

#include "Ui/Win32/WidgetWin32Impl.h"
#include "Ui/Itf/IRadioButton.h"

namespace traktor
{
	namespace ui
	{

class RadioButtonWin32 : public WidgetWin32Impl< IRadioButton >
{
public:
	RadioButtonWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, bool checked);

	virtual void setChecked(bool checked);

	virtual bool isChecked() const;
};

	}
}

#endif	// traktor_ui_RadioButtonWin32_H
