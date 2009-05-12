#ifndef traktor_ui_IPopupMenu_H
#define traktor_ui_IPopupMenu_H

#include "Core/Config.h"
#include "Core/Heap/Ref.h"

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

class MenuItem;
class Point;
class IWidget;

/*! \brief PopupMenu interface.
 * \ingroup UI
 */
class T_DLLCLASS IPopupMenu
{
public:
	virtual bool create() = 0;

	virtual void destroy() = 0;

	virtual void add(MenuItem* item) = 0;

	virtual MenuItem* show(IWidget* parent, const Point& at) = 0;
};

	}
}

#endif	// traktor_ui_IPopupMenu_H
