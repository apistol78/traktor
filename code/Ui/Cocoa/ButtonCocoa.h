#ifndef traktor_ui_ButtonCocoa_H
#define traktor_ui_ButtonCocoa_H

#import "Ui/Cocoa/NSTargetProxy.h"

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/IButton.h"

namespace traktor
{
	namespace ui
	{

class ButtonCocoa
:	public WidgetCocoaImpl< IButton, NSButton >
,	public ITargetProxyCallback
{
public:
	ButtonCocoa(EventSubject* owner);
	
	// IButton

	virtual bool create(IWidget* parent, const std::wstring& text, int style);

	virtual void setState(bool state);

	virtual bool getState() const;
	
	// ITargetProxyCallback
	
	virtual void targetProxy_Action();
};

	}
}

#endif	// traktor_ui_ButtonCocoa_H
