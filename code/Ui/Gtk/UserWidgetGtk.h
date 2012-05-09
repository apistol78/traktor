#ifndef traktor_ui_UserWidgetGtk_H
#define traktor_ui_UserWidgetGtk_H

#include "Ui/Gtk/WidgetGtkImpl.h"
#include "Ui/Itf/IUserWidget.h"

namespace traktor
{
	namespace ui
	{

class UserWidgetGtk : public WidgetGtkImpl< IUserWidget >
{
public:
	UserWidgetGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

private:
	void on_size_allocate(Gtk::Allocation& allocation);
};

	}
}

#endif	// traktor_ui_UserWidgetGtk_H

