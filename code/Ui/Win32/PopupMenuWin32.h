#ifndef traktor_ui_PopupMenuWin32_H
#define traktor_ui_PopupMenuWin32_H

#include "Core/RefArray.h"
#include "Ui/Itf/IPopupMenu.h"
#include "Ui/Win32/Window.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class PopupMenuWin32 : public IPopupMenu
{
public:
	virtual bool create();

	virtual void destroy();

	virtual void add(MenuItem* item);

	virtual MenuItem* show(IWidget* parent, const Point& at);

private:
	RefArray< MenuItem > m_items;
	RefArray< MenuItem > m_flatten;

	HMENU buildMenu(MenuItem* item);
};

	}
}

#endif	// traktor_ui_PopupMenuWin32_H
