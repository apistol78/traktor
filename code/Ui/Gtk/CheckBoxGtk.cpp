#include "Core/Misc/TString.h"
#include "Ui/Gtk/CheckBoxGtk.h"
#include "Ui/Events/ButtonClickEvent.h"

namespace traktor
{
	namespace ui
	{

CheckBoxGtk::CheckBoxGtk(EventSubject* owner)
:	WidgetGtkImpl< ICheckBox >(owner)
{
}

bool CheckBoxGtk::create(IWidget* parent, const std::wstring& text, bool checked)
{
	Internal* parentInternal = static_cast< Internal* >(parent->getInternalHandle());
	T_FATAL_ASSERT(parentInternal);

	Gtk::Fixed* container = new Gtk::Fixed();
	parentInternal->container->put(*container, 0, 0);

	Gtk::CheckButton* button = new Gtk::CheckButton(wstombs(text).c_str());
	button->signal_toggled().connect(sigc::mem_fun(*this, &CheckBoxGtk::on_checkbox_toggled));

	container->put(*button, 0, 0);
	button->show();

	m_internal.container = container;
	m_internal.widget = button;

	return WidgetGtkImpl< ICheckBox >::create();
}

void CheckBoxGtk::setChecked(bool checked)
{
	static_cast< Gtk::CheckButton* >(m_internal.widget)->set_active(checked);
}

bool CheckBoxGtk::isChecked() const
{
	return static_cast< Gtk::CheckButton* >(m_internal.widget)->get_active();
}

void CheckBoxGtk::on_checkbox_toggled()
{
	ButtonClickEvent clickEvent(m_owner);
	m_owner->raiseEvent(&clickEvent);
}

	}
}

