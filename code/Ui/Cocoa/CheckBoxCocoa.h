#ifndef traktor_ui_CheckBoxCocoa_H
#define traktor_ui_CheckBoxCocoa_H

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/ICheckBox.h"

namespace traktor
{
	namespace ui
	{

class CheckBoxCocoa : public WidgetCocoaImpl< ICheckBox, NSButton >
{
public:
	CheckBoxCocoa(EventSubject* owner);
	
	// ICheckBox

	virtual bool create(IWidget* parent, const std::wstring& text, bool checked);

	virtual void setChecked(bool checked);

	virtual bool isChecked() const;
	
	// IWidget
	
	virtual Size getPreferedSize() const;
};

	}
}

#endif	// traktor_ui_CheckBoxCocoa_H
