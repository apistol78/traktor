#include "Core/Misc/TString.h"
#include "Ui/Gtk/ButtonGtk.h"
#include "Ui/Events/ButtonClickEvent.h"

namespace traktor
{
	namespace ui
	{

ButtonGtk::ButtonGtk(EventSubject* owner)
:	WidgetGtkImpl< IButton >(owner)
{
}

bool ButtonGtk::create(IWidget* parent, const std::wstring& text, int style)
{
	Internal* parentInternal = static_cast< Internal* >(parent->getInternalHandle());
	T_FATAL_ASSERT(parentInternal);

	Gtk::Fixed* container = new Gtk::Fixed();
	parentInternal->container->put(*container, 0, 0);

	Gtk::Button* button = new Gtk::Button(wstombs(text).c_str());
	button->signal_clicked().connect(sigc::mem_fun(*this, &ButtonGtk::on_button_clicked));

	container->put(*button, 0, 0);
	button->show();

	m_internal.container = container;
	m_internal.widget = button;

	return WidgetGtkImpl< IButton >::create();
}

void ButtonGtk::setState(bool state)
{
}

bool ButtonGtk::getState() const
{
	return false;
}

void ButtonGtk::on_button_clicked()
{
	ButtonClickEvent clickEvent(m_owner);
	m_owner->raiseEvent(&clickEvent);
}

	}
}

