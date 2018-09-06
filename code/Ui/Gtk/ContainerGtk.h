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

	virtual bool create(IWidget* parent, int style) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_ui_ContainerGtk_H
