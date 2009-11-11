#ifndef traktor_ui_PopupMenu_H
#define traktor_ui_PopupMenu_H

#include "Core/Heap/Ref.h"
#include "Ui/EventSubject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Widget;
class MenuItem;
class Point;
class IPopupMenu;

/*! \brief Popup menu.
 * \ingroup UI
 */
class T_DLLCLASS PopupMenu : public EventSubject
{
	T_RTTI_CLASS(PopupMenu)
	
public:
	PopupMenu();

	virtual ~PopupMenu();

	bool create();

	void destroy();

	void add(MenuItem* item);

	Ref< MenuItem > show(Widget* parent, const Point& at);

private:
	IPopupMenu* m_popupMenu;
};

	}
}

#endif	// traktor_ui_PopupMenu_H
