#ifndef traktor_ui_PopupMenuCocoa_H
#define traktor_ui_PopupMenuCocoa_H

#import <Cocoa/Cocoa.h>
#import "Ui/Cocoa/NSTargetProxy.h"

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

	virtual bool create() T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void add(MenuItem* item) T_OVERRIDE T_FINAL;

	virtual MenuItem* show(IWidget* parent, const Point& at) T_OVERRIDE T_FINAL;
	
	// ITargetProxyCallback
	
	virtual void targetProxy_Action(void* controlId) T_OVERRIDE T_FINAL;
	
	virtual void targetProxy_doubleAction(void* controlId) T_OVERRIDE T_FINAL;

private:
	NSMenu* m_menu;
	Ref< MenuItem > m_selectedItem;
};

	}
}

#endif	// traktor_ui_PopupMenuCocoa_H
