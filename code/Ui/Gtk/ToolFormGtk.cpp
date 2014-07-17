#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/Application.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Gtk/ToolFormGtk.h"

namespace traktor
{
	namespace ui
	{

ToolFormGtk::ToolFormGtk(EventSubject* owner)
:	WindowGtkImpl< IToolForm >(owner)
{
}

bool ToolFormGtk::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	// Create top-level window.
	m_window = new Gtk::Window();
	m_window->set_title(wstombs(text).c_str());
	m_window->set_default_size(width, height);
	m_window->set_border_width(0);

	// Create our fixed child widget container.
	m_container = new Gtk::Fixed();
	m_container->signal_size_allocate().connect(sigc::mem_fun(*this, &ToolFormGtk::on_size_allocate));

	m_window->add(*m_container);

	// Show widgets.
	m_window->show();
	m_container->show();

	return true;
}

void ToolFormGtk::center()
{
}

void ToolFormGtk::on_size_allocate(Gtk::Allocation& allocation)
{
	log::info << L"ToolFormGtk::on_size_allocate : " << allocation.get_width() << L" x " << allocation.get_height() << Endl;

	SizeEvent s(m_owner, Size(allocation.get_width(), allocation.get_height()));
	m_owner->raiseEvent(&s);
}

	}
}

