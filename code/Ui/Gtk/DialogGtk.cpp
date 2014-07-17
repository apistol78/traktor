#include "Core/Misc/TString.h"
#include "Ui/Application.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Gtk/DialogGtk.h"

namespace traktor
{
	namespace ui
	{

DialogGtk::DialogGtk(EventSubject* owner)
:	WindowGtkImpl< IDialog >(owner)
{
}

bool DialogGtk::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	// Create top-level window.
	m_window = new Gtk::Window();
	m_window->set_title(wstombs(text).c_str());
	m_window->set_default_size(width, height);
	m_window->set_border_width(0);

	// Connect signals.
	m_window->signal_remove().connect(sigc::mem_fun(*this, &DialogGtk::on_remove));

	// Create our fixed child widget container.
	m_container = new Gtk::Fixed();
	m_container->signal_size_allocate().connect(sigc::mem_fun(*this, &DialogGtk::on_size_allocate));

	m_window->add(*m_container);

	// Show widgets.
	m_window->show();
	m_container->show();

	return true;
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

void DialogGtk::on_size_allocate(Gtk::Allocation& allocation)
{
	SizeEvent s(m_owner, Size(allocation.get_width(), allocation.get_height()));
	m_owner->raiseEvent(&s);
}

	}
}
