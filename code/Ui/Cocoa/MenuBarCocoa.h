#ifndef traktor_ui_MenuBarCocoa_H
#define traktor_ui_MenuBarCocoa_H

#import <Cocoa/Cocoa.h>

#include "Core/Heap/Ref.h"
#include "Ui/Itf/IMenuBar.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class MenuBarCocoa : public IMenuBar
{
public:
	MenuBarCocoa(EventSubject* owner);

	// IMenuBar

	virtual bool create(IForm* form);

	virtual void destroy();

	virtual void add(MenuItem* item);
	
private:
	NSMenu* m_menu;
};

	}
}

#endif	// traktor_ui_MenuBarCocoa_H
