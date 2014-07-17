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
	// @fixme Not safe, we must check so the internal handle in fact are a Gtk::Fixed container. Also need to ensure cleanup of m_parentContainer.
	m_parentContainer = static_cast< Gtk::Fixed* >(parent->getInternalHandle());
	if (!m_parentContainer)
		return false;

	Gtk::Switch* witch = new Gtk::Switch();
	witch->set_active(checked);
	//witch->signal_clicked().connect(sigc::mem_fun(*this, &CheckBoxGtk::onClicked));

	m_parentContainer->put(*witch, 0, 0);

	witch->show();

	m_widget = witch;

	return true;
}

void CheckBoxGtk::setChecked(bool checked)
{
	static_cast< Gtk::Switch* >(m_widget)->set_active(checked);
}

bool CheckBoxGtk::isChecked() const
{
	return static_cast< Gtk::Switch* >(m_widget)->get_active();
}

void CheckBoxGtk::onClicked()
{
	ButtonClickEvent clickEvent(m_owner);
	m_owner->raiseEvent(&clickEvent);
}

	}
}

