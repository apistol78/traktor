#ifndef traktor_ui_PopupMenuCocoa_H
#define traktor_ui_PopupMenuCocoa_H

#include "Core/Heap/Ref.h"
#include "Ui/Itf/IPopupMenu.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class PopupMenuCocoa : public IPopupMenu
{
public:
	PopupMenuCocoa(EventSubject* owner);
	
	// IPopupMenu

	virtual bool create();

	virtual void destroy();

	virtual void add(MenuItem* item);

	virtual MenuItem* show(IWidget* parent, const Point& at);
};

	}
}

#endif	// traktor_ui_PopupMenuCocoa_H
