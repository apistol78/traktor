#ifndef traktor_ui_StaticGtk_H
#define traktor_ui_StaticGtk_H

#include "Ui/Gtk/WidgetGtkImpl.h"
#include "Ui/Itf/IStatic.h"

namespace traktor
{
	namespace ui
	{

class StaticGtk : public WidgetGtkImpl< IStatic >
{
public:
	StaticGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text);
};

	}
}

#endif	// traktor_ui_StaticGtk_H

