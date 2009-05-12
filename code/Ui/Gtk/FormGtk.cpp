#include "Ui/Gtk/FormGtk.h"
#include "Ui/SizeEvent.h"
#include "Ui/Application.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{

FormGtk::FormGtk(EventSubject* owner)
:	WindowGtkImpl< IForm >(owner)
{
}

bool FormGtk::create(IWidget* parent, const std::string& text, int width, int height, int style)
{
	// Create top-level window.
	m_window = new Gtk::Window();
	m_window->set_title(text.c_str());
	m_window->set_default_size(width, height);
	m_window->set_border_width(0);

	// Connect signals.
	m_window->signal_remove().connect(sigc::mem_fun(*this, &FormGtk::on_remove));

	// Create our fixed child widget container.
	m_container = new Gtk::Fixed();
	m_container->signal_size_allocate().connect(sigc::mem_fun(*this, &FormGtk::on_size_allocate));

	m_window->add(*m_container);

	// Show widgets.
	m_window->show();
	m_container->show();

	return true;
}

void FormGtk::setIcon(drawing::Image* icon)
{
}

IMenuBar* FormGtk::createMenuBar(EventSubject* owner)
{
	return 0;
}

IToolBar* FormGtk::createToolBar(EventSubject* owner)
{
	return 0;
}

IStatusBar* FormGtk::createStatusBar(EventSubject* owner)
{
	return 0;
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

Rect FormGtk::getInnerRect() const
{
	return Rect(0, 0, 0, 0);
}

void FormGtk::on_remove(Gtk::Widget* widget)
{
	Application::getInstance().exit(0);
}

void FormGtk::on_size_allocate(Gtk::Allocation& allocation)
{
	log::info << "FormGtk::on_size_allocate : " << allocation.get_width() << " x " << allocation.get_height() << Endl;

	SizeEvent s(m_owner, Size(allocation.get_width(), allocation.get_height()));
	m_owner->raiseEvent(EiSize, &s);
}

	}
}

