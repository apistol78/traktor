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
	Warp* p = static_cast< Warp* >(parent->getInternalHandle());
	T_FATAL_ASSERT(p != nullptr);

	m_warp.widget = gtk_button_new_with_label(wstombs(text).c_str());
	gtk_container_add(GTK_CONTAINER(p->widget), m_warp.widget);

	m_text = text;

	return WidgetGtkImpl< IButton >::create(parent);
}

void ButtonGtk::setState(bool state)
{
}

bool ButtonGtk::getState() const
{
	return false;
}

void ButtonGtk::setText(const std::wstring& text)
{
	gtk_button_set_label(GTK_BUTTON(m_warp.widget), wstombs(text).c_str());
	WidgetGtkImpl< IButton >::setText(text);
}

void ButtonGtk::on_button_clicked()
{
	ButtonClickEvent clickEvent(m_owner);
	m_owner->raiseEvent(&clickEvent);
}

	}
}

