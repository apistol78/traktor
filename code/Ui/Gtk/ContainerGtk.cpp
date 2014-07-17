#include "Core/Log/Log.h"
#include "Ui/Gtk/ContainerGtk.h"
#include "Ui/Events/SizeEvent.h"

namespace traktor
{
	namespace ui
	{

ContainerGtk::ContainerGtk(EventSubject* owner)
:	WidgetGtkImpl< IContainer >(owner)
{
}

bool ContainerGtk::create(IWidget* parent, int style)
{
	// @fixme Not safe, we must check so the internal handle in fact are a Gtk::Fixed container. Also need to ensure cleanup of m_parentContainer.
	m_parentContainer = static_cast< Gtk::Fixed* >(parent->getInternalHandle());
	if (!m_parentContainer)
		return false;

	// Create container window.
	Gtk::Fixed* container = new Gtk::Fixed();
	container->signal_size_allocate().connect(sigc::mem_fun(*this, &ContainerGtk::on_size_allocate));

	m_parentContainer->put(*container, 0, 0);

	container->show();

	m_widget = container;

	return true;
}

void ContainerGtk::on_size_allocate(Gtk::Allocation& allocation)
{
	log::info << L"ContainerGtk::on_size_allocate : " << allocation.get_width() << L" x " << allocation.get_height() << Endl;

	SizeEvent s(m_owner, Size(allocation.get_width(), allocation.get_height()));
	m_owner->raiseEvent(&s);
}

	}
}

