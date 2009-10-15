#ifndef traktor_ui_CheckBoxCocoa_H
#define traktor_ui_CheckBoxCocoa_H

#import "Ui/Cocoa/NSTargetProxy.h"

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/ICheckBox.h"

namespace traktor
{
	namespace ui
	{

class CheckBoxCocoa
:	public WidgetCocoaImpl< ICheckBox, NSButton >
,	public ITargetProxyCallback
{
public:
	CheckBoxCocoa(EventSubject* owner);
	
	// ICheckBox

	virtual bool create(IWidget* parent, const std::wstring& text, bool checked);

	virtual void setChecked(bool checked);

	virtual bool isChecked() const;
	
	// ITargetProxyCallback
	
	virtual void targetProxy_Action(void* controlId);
	
	virtual void targetProxy_doubleAction(void* controlId);
};

	}
}

#endif	// traktor_ui_CheckBoxCocoa_H
