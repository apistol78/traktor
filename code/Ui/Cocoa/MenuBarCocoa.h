#ifndef traktor_ui_MenuBarCocoa_H
#define traktor_ui_MenuBarCocoa_H

#import <Cocoa/Cocoa.h>
#import "Ui/Cocoa/NSTargetProxy.h"

#include "Core/Heap/Ref.h"
#include "Ui/Itf/IMenuBar.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class MenuBarCocoa
:	public IMenuBar
,	public ITargetProxyCallback
{
public:
	MenuBarCocoa(EventSubject* owner);

	// IMenuBar

	virtual bool create(IForm* form);

	virtual void destroy();

	virtual void add(MenuItem* item);
	
	// ITargetProxyCallback
	
	virtual void targetProxy_Action(void* controlId);
	
	virtual void targetProxy_doubleAction(void* controlId);

private:
	EventSubject* m_owner;
	NSMenu* m_menu;
};

	}
}

#endif	// traktor_ui_MenuBarCocoa_H
