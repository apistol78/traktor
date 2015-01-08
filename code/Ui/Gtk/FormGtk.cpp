#include "Core/Misc/TString.h"
#include "Ui/Application.h"
#include "Ui/Gtk/FormGtk.h"

namespace traktor
{
	namespace ui
	{

FormGtk::FormGtk(EventSubject* owner)
:	WidgetGtkImpl< IForm >(owner)
{
}

bool FormGtk::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	Gtk::Window* window = new Gtk::Window();
	window->set_title(wstombs(text).c_str());
	window->set_default_size(width, height);
	window->set_border_width(0);
	window->signal_remove().connect(sigc::mem_fun(*this, &FormGtk::on_remove));

	window->show();

	Gtk::Fixed* container = new Gtk::Fixed();
	window->add(*container);

	m_internal.container = container;
	m_internal.widget = window;

	return WidgetGtkImpl< IForm >::create();
}

void FormGtk::setIcon(ui::IBitmap* icon)
{
}

void FormGtk::maximize()
{
}

void FormGtk::minimize()
{
}

void FormGtk::restore()
{
}

bool FormGtk::isMaximized() const
{
	return false;
}

bool FormGtk::isMinimized() const
{
	return false;
}

void FormGtk::on_remove(Gtk::Widget* widget)
{
	Application::getInstance()->exit(0);
}

	}
}

