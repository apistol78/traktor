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
	// @fixme Not safe, we must check so the internal handle in fact are a Gtk::Fixed container. Also need to ensure cleanup of m_parentContainer.
	m_parentContainer = static_cast< Gtk::Fixed* >(parent->getInternalHandle());
	if (!m_parentContainer)
		return false;

	Gtk::Button* button = new Gtk::Button(wstombs(text).c_str());
	button->signal_clicked().connect(sigc::mem_fun(*this, &ButtonGtk::on_button_clicked));

	m_parentContainer->put(*button, 0, 0);

	button->show();

	m_widget = button;

	return true;
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

