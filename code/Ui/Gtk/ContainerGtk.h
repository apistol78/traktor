#ifndef traktor_ui_ContainerGtk_H
#define traktor_ui_ContainerGtk_H

#include "Ui/Gtk/WidgetGtkImpl.h"
#include "Ui/Itf/IContainer.h"

namespace traktor
{
	namespace ui
	{

class ContainerGtk : public WidgetGtkImpl< IContainer >
{
public:
	ContainerGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);

private:
	void on_size_allocate(Gtk::Allocation& allocation);
};

	}
}

#endif	// traktor_ui_ContainerGtk_H
