#include "Core/Misc/TString.h"
#include "Ui/Application.h"
#include "Ui/Gtk/DialogGtk.h"

namespace traktor
{
	namespace ui
	{

DialogGtk::DialogGtk(EventSubject* owner)
:	WidgetGtkImpl< IDialog >(owner)
{
}

bool DialogGtk::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	Gtk::Window* window = new Gtk::Window();
	window->set_title(wstombs(text).c_str());
	window->set_default_size(width, height);
	window->set_border_width(0);
	window->signal_remove().connect(sigc::mem_fun(*this, &DialogGtk::on_remove));

	window->show();

	Gtk::Fixed* container = new Gtk::Fixed();
	window->add(*container);

	m_internal.container = container;
	m_internal.widget = window;

	return WidgetGtkImpl< IDialog >::create();
}

void DialogGtk::setIcon(drawing::Image* icon)
{
}

int DialogGtk::showModal()
{
	return DrCancel;
}

void DialogGtk::endModal(int result)
{
}

void DialogGtk::setMinSize(const Size& minSize)
{
}

void DialogGtk::on_remove(Gtk::Widget* widget)
{
	Application::getInstance()->exit(0);
}

	}
}
