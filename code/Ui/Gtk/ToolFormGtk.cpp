#include "Core/Misc/TString.h"
#include "Ui/Application.h"
#include "Ui/Gtk/ToolFormGtk.h"

namespace traktor
{
	namespace ui
	{

ToolFormGtk::ToolFormGtk(EventSubject* owner)
:	WidgetGtkImpl< IToolForm >(owner)
{
}

bool ToolFormGtk::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	Gtk::Window* window = new Gtk::Window();
	window->set_title(wstombs(text).c_str());
	window->set_default_size(width, height);
	window->set_border_width(0);

	window->show();

	Gtk::Fixed* container = new Gtk::Fixed();
	window->add(*container);

	m_internal.container = container;
	m_internal.widget = window;

	return WidgetGtkImpl< IToolForm >::create();
}

void ToolFormGtk::center()
{
}

	}
}

