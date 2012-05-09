#ifndef traktor_ui_ToolFormGtk_H
#define traktor_ui_ToolFormGtk_H

#include "Ui/Gtk/WindowGtkImpl.h"
#include "Ui/Itf/IToolForm.h"

namespace traktor
{
	namespace ui
	{

class ToolFormGtk : public WindowGtkImpl< IToolForm >
{
public:
	ToolFormGtk(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style);

	virtual void center();

private:
	void on_size_allocate(Gtk::Allocation& allocation);
};

	}
}

#endif	// traktor_ui_ToolFormGtk_H

