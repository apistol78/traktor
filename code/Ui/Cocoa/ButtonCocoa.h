#ifndef traktor_ui_ButtonCocoa_H
#define traktor_ui_ButtonCocoa_H

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/IButton.h"

namespace traktor
{
	namespace ui
	{

class ButtonCocoa : public WidgetCocoaImpl< IButton, NSButton >
{
public:
	ButtonCocoa(EventSubject* owner);
	
	// IButton

	virtual bool create(IWidget* parent, const std::wstring& text, int style);

	virtual void setState(bool state);

	virtual bool getState() const;
	
	// IWidget
	
	virtual Size getPreferedSize() const;
};

	}
}

#endif	// traktor_ui_ButtonCocoa_H
