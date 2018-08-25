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
	/*
	GtkWidget* p = static_cast< GtkWidget* >(parent->getInternalHandle());
	T_FATAL_ASSERT(p != nullptr);

	GtkFixed* c = findGtkFixed(p);
	T_FATAL_ASSERT(c != nullptr);





	m_widget = gtk_fixed_new();

	gtk_fixed_put(
		c,
		m_widget,
		0,
		0
	);	


	m_api.container = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	m_api.widget = nullptr;

	gtk_window_set_title(m_api.container, stombs(text).c_str());
	gtk_window_set_default_size(m_api.container, width, height);

	gtk_widget_show_all(m_api.container);



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
	*/
	return false;
}

void DialogGtk::setIcon(ISystemBitmap* icon)
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
/*
void DialogGtk::on_remove(Gtk::Widget* widget)
{
	Application::getInstance()->exit(0);
}
*/
	}
}
