#ifndef traktor_ui_PopupMenuCocoa_H
#define traktor_ui_PopupMenuCocoa_H

#import <Cocoa/Cocoa.h>
#import "Ui/Cocoa/NSTargetProxy.h"

#include "Core/Heap/Ref.h"
#include "Ui/Itf/IPopupMenu.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;
class MenuItem;

class PopupMenuCocoa
:	public IPopupMenu
,	public ITargetProxyCallback
{
public:
	PopupMenuCocoa(EventSubject* owner);
	
	// IPopupMenu

	virtual bool create();

	virtual void destroy();

	virtual void add(MenuItem* item);

	virtual MenuItem* show(IWidget* parent, const Point& at);
	
	// ITargetProxyCallback
	
	virtual void targetProxy_Action(void* controlId);
	
	virtual void targetProxy_doubleAction(void* controlId);

private:
	NSMenu* m_menu;
	Ref< MenuItem > m_selectedItem;
};

	}
}

#endif	// traktor_ui_PopupMenuCocoa_H
