#ifndef traktor_ui_MenuBarGtk_H
#define traktor_ui_MenuBarGtk_H

#include "Ui/Itf/IMenuBar.h"

namespace traktor
{
	namespace ui
	{

class MenuBarGtk : public IMenuBar
{
public:
	virtual bool create(IForm* form);

	virtual void destroy();

	virtual void add(MenuItem* item);
};

	}
}

#endif	// traktor_ui_MenuBarGtk_H
