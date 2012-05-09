#ifndef traktor_ui_PopupMenuGtk_H
#define traktor_ui_PopupMenuGtk_H

#include "Ui/Itf/IPopupMenu.h"

namespace traktor
{
	namespace ui
	{

class PopupMenuGtk : public IPopupMenu
{
public:
	virtual bool create();

	virtual void destroy();

	virtual void add(MenuItem* item);

	virtual MenuItem* show(IWidget* parent, const Point& at);
};

	}
}

#endif	// traktor_ui_PopupMenuGtk_H
